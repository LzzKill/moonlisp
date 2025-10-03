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
import moonlisp.runtime; // 导入整个 runtime 模块

// --- 保留你已有的辅助函数 ---
std::string_view get_string(moonlisp::LexerType lexer_type)
{
  switch (lexer_type) {
  case moonlisp::NUMBER:
    return "NUMBER";
  case moonlisp::NAME:
    return "NAME";
  case moonlisp::STRING:
    return "STRING";
  case moonlisp::SYMBOL:
    return "SYMBOL";
  case moonlisp::_EOF:
    return "_EOF";
  default: ;
  }
  return "UNKNOWN";
}

std::string_view get_string(moonlisp::ast::NodeType node_type)
{
  using moonlisp::ast::NodeType;
  switch (node_type) {
  case NodeType::FLOAT:
    return "FLOAT";
  case NodeType::NUMBER:
    return "NUMBER";
  case NodeType::STRING:
    return "STRING";
  case NodeType::NAME:
    return "NAME";
  case NodeType::DOT:
    return "DOT";
  }
  return "UNKNOWN";
}

inline std::string indent(int depth) { return std::string(depth * 2, ' '); }

template <typename T> struct is_unique_ptr : std::false_type
{
};
template <typename T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type
{
  using pointee = T;
};

template <typename T> decltype(auto) deref(T &x)
{
  if constexpr (is_unique_ptr<std::decay_t<T>>::value) {
    return *x;
  } else {
    return x;
  }
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
        if constexpr (std::is_same_v<U, moonlisp::ast::Atom>) {
          os << get_string(arg.type) << " : " << arg.value << "\n";
        } else if constexpr (std::is_same_v<U, moonlisp::ast::List>) {
          os << "(\n";
          for (const auto &e : arg.elements) {
            viewAST(e, depth + 1, os);
          }
          os << indent(depth) << ")\n";
        } else if constexpr (std::is_same_v<U, moonlisp::ast::Pair>) {
          os << "[\n";
          for (const auto &e : arg.elements) {
            viewAST(e, depth + 1, os);
          }
          os << indent(depth) << "]\n";
        }
      },
      node.node);
}

// --- 新增或修改 main 函数 ---
int main(int argc, char *argv[])
{
  try {
    std::string input_code;

    // --- 方式 1: 从命令行参数读取文件 ---
    if (argc > 1) {
      std::ifstream file(argv[1]);
      if (!file.is_open()) {
        std::cerr << "无法打开文件: " << argv[1] << std::endl;
        return 1;
      }
      std::string line;
      while (std::getline(file, line)) {
        input_code += line + "\n"; // 保留换行符很重要
      }
      file.close();
    } else {
      // --- 方式 2: 从标准输入读取 (交互式或重定向) ---
      std::cout << "请输入 Moonlisp 代码 (输入 EOF 结束):\n";
      std::string line;
      while (std::getline(std::cin, line)) {
        if (line == "EOF")
          break;
        input_code += line + "\n";
      }
    }

    if (input_code.empty()) {
      std::cerr << "没有输入代码。" << std::endl;
      return 1;
    }

    std::cout << "--- 输入的代码 ---\n" << input_code << "--- 结束 ---\n";

    // 1. 创建 Lexer
    auto lexer = std::make_unique<moonlisp::Lexer>(input_code);

    // 2. 创建 Parser (Parser 会自动调用 lexer 获取 token)
    auto parser = std::make_unique<moonlisp::Parser>(std::move(lexer));
    moonlisp::ast::TopNode ast = parser->getAST(); // 获取解析后的 AST

    std::cout << "\n--- 生成的 AST ---\n";
    for (const auto &node : ast) {
      viewAST(node); // 使用你已有的函数打印 AST
    }
    std::cout << "--- AST 结束 ---\n";

    // 3. 设置运行环境
    // 创建一个全局环境，并加载内置函数 (如 print)
    auto global_env = moonlisp::runtime::builtin::getBuiltinEnvironment();

    // 4. 执行 (Eval)
    std::cout << "\n--- 开始执行 ---\n";
    for (const auto &node : ast) {
      try {
        // 调用你的 eval 函数
        moonlisp::runtime::Value_p result =
            moonlisp::runtime::eval(node, global_env);

        // (可选) 打印或处理结果
        // 你需要一个函数来将 Value_p 转换为可读字符串
        // 例如: std::cout << "Result: " << valueToString(result) << std::endl;
        // 暂时可以只打印执行了
        //std::cout << "Executed expression.\n";
      }
      catch (const moonlisp::RuntimeError &e) {
        e.show();
        // 可以选择继续执行下一条或退出
        // return 1;
      }
      catch (const std::exception &e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        // return 1;
      }
    }
    std::cout << "\n --- 执行结束 ---\n";
  }
  catch (const moonlisp::LexerError &e) {
    e.show();
    return 1;
  }
  catch (const moonlisp::ParserError &e) {
    e.show();
    return 1;
  }
  catch (const std::exception &e) {
    std::cerr << "未捕获的标准异常: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "未捕获的未知异常!" << std::endl;
    return 1;
  }

  return 0;
}