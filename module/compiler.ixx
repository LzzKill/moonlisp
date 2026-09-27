/*
 * Code: compiler.ixx
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 * 编译器实现
 * */

module;
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
export module moonlisp.compiler;

import moonlisp.value;
import moonlisp.ast;
import moonlisp.parser;
import moonlisp.exception;
import moonlisp.constant;

using moonlisp::ast::Node;


export namespace moonlisp
{
  using Instruction_v = std::vector<Instruction>;

  class Compiler {

    ast::TopNode ast_node;
    Instruction_v instructions;

    inline void emit(const ByteCodeVM opcode) { this->instructions.emplace_back(opcode); }
    inline void emit(const ByteCodeVM opcode, Value_p arg) { this->instructions.emplace_back(opcode, arg); }

    void compiler()
    {
      for (const auto &node : ast_node) compileNode(node);
    }

    void compileNode(const ast::Node &);

    void compileAtom(const ast::Node &);

    void compileList(const ast::Node &);

    // void compilePair(const ast::Node &);

    void compileQuote(const ast::Node &);

    void compileIf(const ast::Node &);

    void compileLambda(const ast::Node &);

  public:
    explicit Compiler(const std::unique_ptr<Parser> &parser) : ast_node(std::move(parser->getAST())) { compiler(); }

    explicit Compiler(const ast::TopNode &node) : ast_node(node) { compiler(); }


    [[nodiscard]] const Instruction_v &getInstructions() const { return instructions; }
  };


} // namespace moonlisp


void moonlisp::Compiler::compileNode(const ast::Node &node)
{
  std::visit(
      [&]<typename T0>(const T0 &node_ptr) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, ast::Atom_p>) { this->compileAtom(node); }
        else if constexpr (std::is_same_v<T, ast::List_p>) { this->compileList(node); }
        // else if constexpr (std::is_same_v<T, ast::Pair_p>) { this->compilePair(node); }
        else { throw CompilerError(node.place, "Unknown node type"); }
      },
      node.node);
}

void moonlisp::Compiler::compileAtom(const ast::Node &node)
{

  const auto &atom = std::get<ast::Atom_p>(node.node);
  switch (atom->type)
  {
    case ast::NodeType::NUMBER: {
      emit(ByteCodeVM::PUSH_CONST, util::make_number(std::stoi(atom->value)));
      break;
    }
    case ast::NodeType::FLOAT: {
      emit(ByteCodeVM::PUSH_CONST, util::make_float(std::stod(atom->value)));
      break;
    }
    case ast::NodeType::STRING: /*{
      emit(ByteCodeVM::PUSH_CONST, util::make_string(atom->value));
      break;
    }*/
    case ast::NodeType::NAME: {
      // 推送变量值
      emit(ByteCodeVM::PUSH_VAR, util::make_string(atom->value));
      break;
    }
    default: throw CompilerError(node.place, std::format("Unknown atom type: {}", atom->value));
  }
}

void moonlisp::Compiler::compileList(const ast::Node &node)
{
  const auto &list = std::get<ast::List_p>(node.node);

  if (!list) { return; }
  if (list->elements.empty()) { return; }

  // 检查是否是特殊形式（如定义、条件等）
  const auto &first = list->elements[0];
  std::string symbol;
  if (std::holds_alternative<ast::Atom_p>(first.node))
  {
    const auto &atom = std::get<ast::Atom_p>(first.node);
    symbol = atom->value;
  }
  if (symbol == "if")
  {
    if (list->elements.size() < 3) { throw CompilerError(node.place, "if requires at least 2 arguments"); }
    compileIf(node);
    return;
  }
  if (symbol == "quote") // TODO: For '
  {
    if (list->elements.size() != 2) { throw CompilerError(node.place, "quote requires exactly one argument"); }
    compileQuote(list->elements[1]);
    return;
  }
  if (symbol == "lambda")
  {
    // lambda 至少要有：lambda 参数列表 体表达式(>=1)
    if (list->elements.size() < 3)

      throw CompilerError(node.place, "lambda requires at least parameter list and one body expression");

    compileLambda(node);
    return;
  }

  // 普通函数调用：先编译所有参数，再编译函数，最后调用
  for (size_t i = 1; i < list->elements.size(); ++i) { compileNode(list->elements[i]); }
  compileNode(first);
  instructions.emplace_back(ByteCodeVM::CALL, list->elements.size() - 1); // 操作数是参数数量
}

void moonlisp::Compiler::compileQuote(const ast::Node &node)
{
  std::visit(
      [&]<typename T0>(const T0 &node_ptr) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, ast::Atom_p>)

          emit(ByteCodeVM::MAKE_SYMBOL, util::make_string(node_ptr->value)); // [1]

        // else if constexpr (std::is_same_v<T, ast::List_p>) // NOTE: lisp允许quote后跟S表达式，但这里临时忽略
        //{
        //   // 对于列表，递归编译所有元素，然后创建列表
        //   const auto &list = *node_ptr;
        //   size_t element_count = list.elements.size();

        //  // 从后往前编译，以便在栈上形成正确的顺序供 MAKE_LIST 使用
        //  for (auto it = list.elements.rbegin(); it != list.elements.rend(); ++it) compileQuote(*it); //
        //  递归处理每个元素 emit(MAKE_LIST, element_count);
        //}
        // else if constexpr (std::is_same_v<T, ast::Pair_p>)
        //{
        //  // 对于Pair的处理，类似列表
        //  const auto &pair = *node_ptr;
        //  size_t element_count = pair.elements.size();

        //  // 从后往前编译
        //  for (auto it = pair.elements.rbegin(); it != pair.elements.rend(); ++it)
        //    compileQuote(*it); // 递归处理每个元素

        //  instructions.emplace_back(MAKE_PAIR, element_count);
        //}
        else
          static_assert("Unknown");
      },
      node.node);
}

void moonlisp::Compiler::compileIf(const ast::Node &node)
{
  // (if cond true [false])
  const auto &list = std::get<ast::List_p>(node.node);

  // 编译条件
  compileNode(list->elements[1]);

  // 1. 生成 JUMP_IF_FALSE，占位，暂不填地址
  size_t jumpIfFalseIdx = instructions.size();
  instructions.emplace_back(ByteCodeVM::JUMP_IF_FALSE, 0);

  // 编译true分支
  compileNode(list->elements[2]);

  // 2. 生成true分支结束后的无条件跳转，占位
  size_t jumpPastElseIdx = instructions.size();
  instructions.emplace_back(ByteCodeVM::JUMP, 0);

  // 此时，else分支的起点就是现在的instructions.size()
  size_t elseStart = instructions.size();
  instructions[jumpIfFalseIdx].operand->data = static_cast<int>(elseStart);

  // 编译else分支（可选）
  if (list->elements.size() == 4) { compileNode(list->elements[3]); }

  // if整体结束位置
  size_t ifEnd = instructions.size();
  instructions[jumpPastElseIdx].operand->data = static_cast<int>(ifEnd);
}


//// 在 moonlisp::Compiler 类中
// void moonlisp::Compiler::compilePair(const ast::Node &node)
//{
//   const auto &pair_ast = *std::get<ast::Pair_p>(node.node);
//   for (const auto &elem : pair_ast.elements) { compileNode(elem); }
//   emit(MAKE_PAIR, pair_ast.elements.size()); // 如果是空的，自然会push 0
// }

void moonlisp::Compiler::compileLambda(const ast::Node &lambda_node)
{
  const auto &list_node = *std::get<ast::List_p>(lambda_node.node);
  const auto &params_node = list_node.elements[1];
  // body 是 elements[2] 直到末尾所有节点，隐式progn
  ast::TopNode body_ast;
  for (size_t i = 2; i < list_node.elements.size(); i++) { body_ast.push_back(list_node.elements[i]); }

  std::vector<std::string> param_names;
  if (std::holds_alternative<ast::List_p>(params_node.node))
  {
    const auto &params_list = *std::get<ast::List_p>(params_node.node);
    for (const auto &param_elem : params_list.elements)
    {
      if (std::holds_alternative<ast::Atom_p>(param_elem.node))
      {
        const auto &atom = *std::get<ast::Atom_p>(param_elem.node);
        if (atom.type == ast::NodeType::NAME) { param_names.push_back(atom.value); }
        else { throw CompilerError(param_elem.place, "lambda parameters must be symbol names"); }
      }
      else { throw CompilerError(param_elem.place, "lambda parameters must be symbols"); }
    }
  }
  else { throw CompilerError(params_node.place, "lambda parameters must be a list"); }

  // 子编译器编译整个body序列（多个表达式，隐式progn）
  Compiler body_compiler(body_ast);
  auto body_byte = body_compiler.getInstructions();
  body_byte.emplace_back(ByteCodeVM::RETURN);

  // 构造lambda值，env=nullptr，运行时PUSH_LAMBDA捕获当前env
  auto lam_val = util::make_lambda(nullptr, std::move(param_names), std::move(body_byte));
  emit(ByteCodeVM::PUSH_LAMBDA, lam_val);
}
