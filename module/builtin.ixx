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

export module moonlisp.runtime.builtin;

import moonlisp.runtime.value;
import moonlisp.exception;

export namespace moonlisp::runtime::builtin
{
  // 修正NativeFunction的参数类型
  using NativeFunction = std::function<Value_p(const std::vector<Value_p>&, Env_p)>;

  Value_p print(const std::vector<Value_p>& args, const Env_p& env) {
    for (const auto& arg : args) {
      std::visit([](auto&& data) {
        std::cout << data;
      }, arg->data);
    }
    std::cout << std::endl;
    return nullptr;
  }

  Value_p add(const std::vector<Value_p>& args, const Env_p& env) {
    double result = 0;
    for (const auto& arg : args) {
      result += std::get<double>(arg->data);
    }
    return util::make_number(result);
  }

  Value_p minus(const std::vector<Value_p>& args, const Env_p& env) {
    if (args.empty()) {
      throw RuntimeError({}, "minus requires at least one argument");
    }
    double result = std::get<double>(args[0]->data);
    for (size_t i = 1; i < args.size(); ++i) {
      result -= std::get<double>(args[i]->data);
    }
    return util::make_number(result);
  }

  Value_p multiply(const std::vector<Value_p>& args, const Env_p& env) {
    double result = 1;
    for (const auto& arg : args) {
      result *= std::get<double>(arg->data);
    }
    return util::make_number(result);
  }

  Value_p div(const std::vector<Value_p>& args, const Env_p& env) {
    if (args.empty()) {
      throw RuntimeError({}, "div requires at least one argument");
    }
    double result = std::get<double>(args[0]->data);
    for (size_t i = 1; i < args.size(); ++i) {
      double val = std::get<double>(args[i]->data);
      if (val == 0) {
        throw RuntimeError({}, "Division by zero");
      }
      result /= val;
    }
    return util::make_number(result);
  }

  Value_p load(const std::vector<Value_p>& args, const Env_p& env) {
    // 实现加载外部库的逻辑
    return nullptr;
  }

  Value_p _import(const std::vector<Value_p>& args, const Env_p& env) {
    // 实现导入模块的逻辑
    return nullptr;
  }

  Value_p set(const std::vector<Value_p>& args, const Env_p& env) {
    if (args.size() != 2) {
      throw RuntimeError({}, "'set' requires two arguments");
    }

    std::string name = std::get<std::string>(std::get<Symbol>(args[0]->data).data->data);
    env->setLocal(name, args[1]);
    return args[1];
  }

  std::shared_ptr<Environment> getBuiltinEnvironment() {
    auto env = std::make_shared<Environment>();
    // 修正函数绑定，之前全部绑定到了print是错误的
    env->setGlobal("print", util::make_native(print));
    env->setGlobal("+", util::make_native(add));
    env->setGlobal("-", util::make_native(minus));
    env->setGlobal("*", util::make_native(multiply));
    env->setGlobal("/", util::make_native(div));
    env->setGlobal("load", util::make_native(load));
    env->setGlobal("import", util::make_native(_import));
    env->setGlobal("set!", util::make_native(set));
    return env;
  }

} // namespace moonlisp::runtime::builtin
