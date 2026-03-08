/*
 * Code: builtin.ixx
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 * 内置函数实现
 * */
module;

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <variant>

export module moonlisp.builtin;

import moonlisp.value;
import moonlisp.exception;

namespace moonlisp::builtin
{

  using arg_p = std::vector<Value_p>;

  // 辅助函数：递归打印单个 Value_p 对象
  void print_single_value(const Value_p &val_p)
  {
    if (!val_p)
    {
      std::cout << "nil"; // 如果是空指针，打印 nil
      return;
    }

    std::visit([&]<typename T0>(T0 &&arg) {
      using T = std::decay_t<T0>;
      // 处理基本类型：int (来自 make_float), double (来自 make_number), std::string
      if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, std::string>)
      {
        std::cout << arg;
      }
      // 处理 ListValue
      else if constexpr (std::is_same_v<T, ConsCell>)
      {
        std::cout << "(";
        bool first = true;
        for (const auto &element : arg.data)
        {
          if (!first) std::cout << " ";
          print_single_value(element); // 递归打印列表元素
          first = false;
        }
        std::cout << ")";
      }
      // 处理 PairValue
      else if constexpr (std::is_same_v<T, PairValue>)
      {
        std::cout << "[";
        bool first = true;
        for (const auto &element : arg.data)
        {
          if (!first) std::cout << " ";
          print_single_value(element); // 递归打印对的元素
          first = false;
        }
        std::cout << "]";
      }
      // 处理 Symbol
      else if constexpr (std::is_same_v<T, Symbol>)
      {
        std::cout << "SYMBOL: ";
        print_single_value(arg.data);
      }
      // 处理 Lambda, Macro, NativeFunction (通常打印它们的特殊表示)
      else if constexpr (std::is_same_v<T, Lambda>) { std::cout << "#<lambda>"; }
      else if constexpr (std::is_same_v<T, Macro>) { std::cout << "#<macro>"; }
      else if constexpr (std::is_same_v<T, NativeFunction>) { std::cout << "#<native-function>"; }
      // 默认处理未知类型（理论上 std::variant 应该覆盖所有情况）
      else { std::cout << "#<unknown-type>"; }
    }, val_p->data); // 假设 Value_p 指向的 Value 结构体有一个名为 'data' 的成员
  }

  Value_p print(const arg_p &args, Env_p &env)
  {
    // 遍历所有传入 print 函数的参数
    for (size_t i = 0; i < args.size(); ++i)
    {
      print_single_value(args[i]);
      if (i < args.size() - 1) { std::cout << " "; }
    }
    return nullptr;
  }

  Value_p exit(const arg_p &arg, Env_p &env) // Exit 不是标准异常，仅仅退出VM，不需要继承std::exception
  {
    if (arg.size() > 1) throw Exit(-1);
    if (arg.size() == 0) throw Exit(0);
    const auto &m = arg[0];
    if (std::holds_alternative<int>(m->data)) throw Exit(std::get<int>(m->data));
    throw Exit(-2);
    return nullptr;
  }

} // namespace moonlisp::runtime::builtin

export namespace moonlisp
{
  Env_p getBuiltin()
  {
    auto p = std::make_shared<Environment>();
    p->setLocal("print", util::make_native(builtin::print));
    p->setLocal("exit", util::make_native(builtin::exit));
    return p;
  }
}
