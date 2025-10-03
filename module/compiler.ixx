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

import moonlisp.runtime.value;
import moonlisp.ast;
import moonlisp.exception;
import moonlisp.constant;

using moonlisp::ast::Node;
using Operand = std::variant<std::string, double, int, size_t>;

export namespace moonlisp
{
  // 字节码指令集
  enum ByteCode : uint8_t {
    NOP = 0x00, // 空操作
    POP, // 弹出栈顶元素
    PUSH_VALUE, // 推送一个值到栈上
    PUSH_VARIABLE, // 推送全局变量到栈上
    PUSH_SYMBOL, // 推送一个符号，即 quote
    CALL, // 调用函数
    JUMP, // 无条件跳转
    JUMP_IF_FALSE, // 条件跳转（如果为假）
    HALT // 停止执行
  };

  // 指令结构：操作码 + 可选操作数
  struct Instruction {
    ByteCode op;
    std::optional<Operand> operand;

    explicit Instruction(const ByteCode opcode) :
      op(opcode), operand(std::nullopt) { }

    explicit Instruction(const ByteCode opcode, const Operand &opnd) :
      op(opcode), operand(opnd) { }
  };

  class Compiler {
    using Instruction_v = std::vector<Instruction>;
    ast::TopNode ast_node;
    Instruction_v instructions;

    void compiler();

    void compileNode(const ast::Node &);

    void compileAtom(const ast::Node &);

    void compileList(const ast::Node &);

    // 编译Pair节点
    void compilePair(const ast::Pair &pair)
    {
      // 对于Pair的处理，这里简化实现
      for (const auto &elem : pair.elements) { compileNode(elem); }
      // 可以根据需要添加特定的Pair处理逻辑
    }

  public:
    explicit Compiler(ast::TopNode top_ast) :
      ast_node(std::move(top_ast)) { compiler(); }

    [[nodiscard]] const Instruction_v &getInstructions() const { return instructions; }
  };


} // namespace moonlisp
void moonlisp::Compiler::compiler()
{
  for (const auto &node : ast_node) compileNode(node);
  instructions.emplace_back(HALT);
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

  // 处理条件语句
  if (symbol == "if")
  {
    /*
     * (if (cond.) (true))
     * (if (cond.) (true) (false))
     */
    if (list->elements.size() < 3) { throw CompilerError(node.place, "if requires at least 2 arguments"); }

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
    return;
  }
  if (symbol == "quote" || symbol == "'")
  {
    /*
     * (quote symbol)
     * 'symbol
     */
    // 递归编译被quote的表达式，但不作为代码执行，而是作为数据
    // std::visit([&](const auto& node_ptr) {
    //   using T = std::decay_t<decltype(node_ptr)>;
    //
    //   if constexpr (std::is_same_v<T, ast::Atom_p>) {
    //     // 对于原子，创建符号
    //     const auto& atom = *node_ptr;
    //     instructions.emplace_back(PUSH_VALUE, atom.value);
    //     instructions.emplace_back(MAKE_SYMBOL);
    //   }
    //   else if constexpr (std::is_same_v<T, ast::List_p>) {
    //     // 对于列表，先编译所有元素，再创建列表
    //     const auto& list = *node_ptr;
    //     size_t element_count = list.elements.size();
    //
    //     for (const auto& elem : list.elements) {
    //       compileQuote(elem);  // 递归处理每个元素
    //     }
    //
    //     // 创建列表
    //     instructions.emplace_back(MAKE_LIST, element_count);
    //   }
    //   else if constexpr (std::is_same_v<T, ast::Pair_p>) {
    //     // 对于Pair的处理，类似列表
    //     const auto& pair = *node_ptr;
    //     size_t element_count = pair.elements.size();
    //
    //     for (const auto& elem : pair.elements) {
    //       compileQuote(elem);  // 递归处理每个元素
    //     }
    //
    //     // 可以使用MAKE_LIST或专门的MAKE_PAIR指令
    //     instructions.emplace_back(MAKE_LIST, element_count);
    //   }
    // }, quoted_node.node);
  }


  // 普通函数调用：先编译所有参数，再编译函数，最后调用
  for (size_t i = 1; i < list->elements.size(); ++i) { compileNode(list->elements[i]); }
  compileNode(first);
  instructions.emplace_back(CALL, list->elements.size() - 1); // 操作数是参数数量
}
