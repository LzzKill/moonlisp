/*
 * Code: main.cpp
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

#include <cstdlib>
#include <exception>
#include <format>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

// 导入你的模块
import moonlisp.constant;
import moonlisp.lexer;
import moonlisp.parser;
import moonlisp.ast;
import moonlisp.exception;
import moonlisp.compiler;
import moonlisp.value;

// --- 保留你已有的辅助函数 ---
std::string_view get_string(moonlisp::LexerType lexer_type)
{
  switch (lexer_type)
  {
    case moonlisp::NUMBER: return "NUMBER";
    case moonlisp::NAME: return "NAME";
    case moonlisp::STRING: return "STRING";
    case moonlisp::SYMBOL: return "SYMBOL";
    case moonlisp::_EOF: return "_EOF";
    default: ;
  }
  return "UNKNOWN";
}

std::string_view get_string(moonlisp::ast::NodeType node_type)
{
  using moonlisp::ast::NodeType;
  switch (node_type)
  {
    case NodeType::FLOAT: return "FLOAT";
    case NodeType::NUMBER: return "NUMBER";
    case NodeType::STRING: return "STRING";
    case NodeType::NAME: return "NAME";
    case NodeType::DOT: return "DOT";
  }
  return "UNKNOWN";
}

inline std::string indent(int depth) { return std::string(depth * 2, ' '); }

template<typename T>
struct is_unique_ptr : std::false_type { };

template<typename T>
struct is_unique_ptr<std::unique_ptr<T> > : std::true_type {
  using pointee = T;
};

template<typename T>
decltype(auto) deref(T &x)
{
  if constexpr (is_unique_ptr<std::decay_t<T> >::value) { return *x; }
  else { return x; }
}

// --- 保留你的 viewAST 函数 ---
inline void viewAST(const moonlisp::ast::Node &node, int depth = 0,
                    std::ostream &os = std::cout)
{
  os << indent(depth);
  std::visit(
      [&](auto const &arg_raw) {
        auto const &arg = deref(arg_raw);
        using U = std::decay_t<decltype(arg)>;
        // os << get_string(arg.type) << " ";
        if constexpr (std::is_same_v<U, moonlisp::ast::Atom>)
        {
          os << get_string(arg.type) << " : " << arg.value << "\n";
        }
        else if constexpr (std::is_same_v<U, moonlisp::ast::List>)
        {
          os << "(\n";
          for (const auto &e : arg.elements) { viewAST(e, depth + 1, os); }
          os << indent(depth) << ")\n";
        }
        else if constexpr (std::is_same_v<U, moonlisp::ast::Pair>)
        {
          os << "[\n";
          for (const auto &e : arg.elements) { viewAST(e, depth + 1, os); }
          os << indent(depth) << "]\n";
        }
      },
      node.node);
}

std::string_view printByteCode(moonlisp::ByteCode code)
{
  switch (code)
  {
    case moonlisp::NOP: break;
    case moonlisp::POP: return "POP";
    case moonlisp::PUSH_VALUE: return "PUSH_VALUE";
    case moonlisp::PUSH_VARIABLE: return "PUSH_VARIABLE";
    case moonlisp::MAKE_LIST: return "MAKE_LIST";
    case moonlisp::MAKE_PAIR: return "MAKE_PAIR";
    case moonlisp::CALL: return "CALL";
    case moonlisp::JUMP: return "JUMP";
    case moonlisp::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
    case moonlisp::HALT: return "HALT";
  }
  return "ERROR";
}


template<typename T> constexpr bool always_false = false;

std::string printArgs(std::optional<std::variant<std::string, double, int, size_t> > operand
    )
{

  if (operand == std::nullopt) return "nullptr";
  return std::visit([&]<typename T0>(const T0& m) -> std::string {
    using T = std::decay_t<T0>;
    if constexpr(std::is_same_v<T, double>) return std::to_string(m);
    else if constexpr(std::is_same_v<T, int>) return std::to_string(m);
    else if constexpr(std::is_same_v<T, size_t>) return std::to_string(m);
    else if constexpr(std::is_same_v<T, std::string>) return m;
    else
      static_assert(always_false<T>, "Error in compiler of C++");
    return "ERROR";
  },operand.value());
}


// --- 新增或修改 main 函数 ---

int main()
{
  std::cout << "请输入 Moonlisp 代码 (输入 EOF 结束):\n";
  std::string input_code;
  std::string line;
  while (std::getline(std::cin, line))
  {
    if (line == "EOF") break;
    input_code += line + "\n";
  }

  if (input_code.empty())
  {
    std::cerr << "没有输入代码。" << std::endl;
    return 1;
  }

  std::cout << "--- 输入的代码 ---\n" << input_code << "--- 结束 ---\n";

  try
  {
    // 1. 创建 Lexer (词法分析器)
    // 引用自 main.cpp [9]
    auto lexer = std::make_unique<moonlisp::Lexer>(input_code);
    auto parser = std::make_unique<moonlisp::Parser>(std::move(lexer));
    auto compiler = std::make_unique<moonlisp::Compiler>(std::move(parser));
    const auto &m = compiler->getInstructions();
    for (int i = 0; i < m.size(); ++i)
    {
      std::cout << i << " : " << printByteCode(m[i].op) << " Args: " << printArgs(m[i].operand) << "\n";
    }
    std::cout << std::flush;
  } catch (const moonlisp::LexerError &e)
  { // 引用自 main.cpp [9]
    e.show();
    return 1;
  } catch (const moonlisp::ParserError &e)
  { // 引用自 main.cpp [9]
    e.show();
    return 1;
  } catch (const moonlisp::CompilerError &e)
  {
    e.show();
    return 1;
  } catch (const std::exception &e)
  { // 引用自 main.cpp [9]
    std::cerr << "未捕获的标准异常: " << e.what() << std::endl;
    return 1;
  } catch (...)
  { // 引用自 main.cpp [9]
    std::cerr << "未捕获的未知异常!" << std::endl;
    return 1;
  }

  return 0;
}
