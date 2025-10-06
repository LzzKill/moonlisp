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

    void compiler();

    void compileNode(const ast::Node &);

    void compileAtom(const ast::Node &);

    void compileList(const ast::Node &);

    void compilePair(const ast::Node &);

    void compileQuote(const ast::Node &);

    void compileIf(const ast::Node &);

    void compileLambda(const ast::Node &);

  public:
    explicit Compiler(const std::unique_ptr<Parser> &parser) :
      ast_node(std::move(parser->getAST())) { compiler(); }

    explicit Compiler(const ast::Node &node) :
      ast_node({ node }) { compiler(); }


    [[nodiscard]] const Instruction_v &getInstructions() const { return instructions; }
  };


} // namespace moonlisp
void moonlisp::Compiler::compiler()
{
  for (const auto &node : ast_node) compileNode(node);
  // 理论上添加halt指令，halt指令移交vm
}

void moonlisp::Compiler::compileNode(const ast::Node &node)
{
  std::visit([&]<typename T0>(const T0 &node_ptr) {
    using T = std::decay_t<T0>;

    if constexpr (std::is_same_v<T, ast::Atom_p>) { this->compileAtom(node); }
    else if constexpr (std::is_same_v<T, ast::List_p>) { this->compileList(node); }
    else if constexpr (std::is_same_v<T, ast::Pair_p>) { this->compilePair(node); }
    else { throw CompilerError(node.place, "Unknown node type"); }
  }, node.node);
}

void moonlisp::Compiler::compileAtom(const ast::Node &node)
{

  const auto &atom = std::get<ast::Atom_p>(node.node);

  switch (atom->type)
  {
    case ast::NodeType::NUMBER: this->instructions.emplace_back(PUSH_VALUE, std::stoi(atom->value));
      break;
    case ast::NodeType::FLOAT: {
      instructions.emplace_back(PUSH_VALUE, std::stod(atom->value));
      break;
    }
    case ast::NodeType::STRING: {
      instructions.emplace_back(PUSH_VALUE, atom->value);
      break;
    }
    case ast::NodeType::NAME: {
      // 推送变量值
      instructions.emplace_back(PUSH_VARIABLE, atom->value);
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
    /*
     * (if (cond.) (true))
     * (if (cond.) (true) (false))
     */
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
    // (lambda (args) (body...))
    if (list->elements.size() != 3) { throw CompilerError(node.place, "lambda required argument is 3"); }
    compileLambda(node);
    return;
  }

  // 普通函数调用：先编译所有参数，再编译函数，最后调用
  for (size_t i = 1; i < list->elements.size(); ++i) { compileNode(list->elements[i]); }
  compileNode(first);
  instructions.emplace_back(CALL, list->elements.size() - 1); // 操作数是参数数量
}

void moonlisp::Compiler::compileQuote(const ast::Node &node)
{
  std::visit([&]<typename T0>(const T0 &node_ptr) {
    using T = std::decay_t<T0>;

    if constexpr (std::is_same_v<T, ast::Atom_p>)
    {
      const auto &atom = *node_ptr;
      if (atom.type == ast::NodeType::NAME)
      {
        instructions.emplace_back(MAKE_SYMBOL, atom.value); // [1]
      }
      else
      {
        instructions.emplace_back(PUSH_VALUE, atom.value); // [1]
      }
    }
    else if constexpr (std::is_same_v<T, ast::List_p>)
    {
      // 对于列表，递归编译所有元素，然后创建列表
      const auto &list = *node_ptr;
      size_t element_count = list.elements.size();

      // 从后往前编译，以便在栈上形成正确的顺序供 MAKE_LIST 使用
      for (auto it = list.elements.rbegin(); it != list.elements.rend(); ++it)
      {
        compileQuote(*it); // 递归处理每个元素
      }

      instructions.emplace_back(MAKE_LIST, element_count);
    }
    else if constexpr (std::is_same_v<T, ast::Pair_p>)
    {
      // 对于Pair的处理，类似列表
      const auto &pair = *node_ptr;
      size_t element_count = pair.elements.size();

      // 从后往前编译
      for (auto it = pair.elements.rbegin(); it != pair.elements.rend(); ++it)
      {
        compileQuote(*it); // 递归处理每个元素
      }

      instructions.emplace_back(MAKE_PAIR, element_count);
    }
  }, node.node);
}

void moonlisp::Compiler::compileIf(const ast::Node &node)
{
  const auto &list = std::get<ast::List_p>(node.node);
  compileNode(list->elements[1]); // cond.

  auto else_jump_pos = instructions.size();
  instructions.emplace_back(JUMP_IF_FALSE, else_jump_pos);

  compileNode(list->elements[2]); // true

  // 生成跳转到结尾的指令
  auto end_jump_pos = instructions.size();
  instructions.emplace_back(JUMP, end_jump_pos);

  // 填充else分支的跳转目标
  instructions[else_jump_pos].operand = instructions.size();

  // 编译else分支（如果存在）
  if (list->elements.size() > 3) { compileNode(list->elements[3]); }

  // 填充结尾跳转的目标
  instructions[end_jump_pos].operand = instructions.size();
}

// 在 moonlisp::Compiler 类中
void moonlisp::Compiler::compilePair(const ast::Node &node)
{
  const auto &pair_ast = *std::get<ast::Pair_p>(node.node);

  // 检查 Pair 是否为空，如果为空，可以根据需求推送一个特定的空值，
  // 或者抛出错误，这里我们假设空Pair也是合法的，但可能需要特殊处理。
  if (pair_ast.elements.empty())
  {
    instructions.emplace_back(MAKE_PAIR, 0); // 创建一个空Pair
    return;
  }

  for (const auto &elem : pair_ast.elements) { compileNode(elem); }
  instructions.emplace_back(MAKE_PAIR, pair_ast.elements.size());
}

void moonlisp::Compiler::compileLambda(const ast::Node &lambda_node)
{
  const auto &list_node = *std::get<ast::List_p>(lambda_node.node);

  const auto &params_node = list_node.elements[1]; // 参数列表 AST 节点
  const auto &body_node = list_node.elements[2];

  std::vector<std::string> param_names;
  if (std::holds_alternative<ast::List_p>(params_node.node))
  {
    const auto &params_list = *std::get<ast::List_p>(params_node.node);
    for (const auto &param_elem : params_list.elements)
    {
      if (std::holds_alternative<ast::Atom_p>(param_elem.node))
      {
        const auto &atom = *std::get<ast::Atom_p>(param_elem.node);
        if (atom.type == ast::NodeType::NAME)
        {
          param_names.push_back(atom.value); // 参数必须是List，List中必须包含NAME项，别的都不行
        }
        else { throw CompilerError(param_elem.place, "lambda parameters must be symbols"); }
      }
      else { throw CompilerError(param_elem.place, "lambda parameters must be symbols"); }
    }
  }
  else { throw CompilerError(params_node.place, "lambda parameters must be a list"); }

  // 编译 lambda结构体，使用新的 Compiler 实例

  auto t_compiler = Compiler(body_node); // 不使用指针分配
  auto byte = t_compiler.getInstructions();
  byte.emplace_back(RETURN); // 添加末尾指令
  /*
   * 此处实现争议较大。
   * 我选择直接push编译好的lambda实例，VM知道如何处理env
   */
  this->instructions.emplace_back(PUSH_LAMBDA, Lambda{ nullptr, param_names, byte }); // 留空env
}
