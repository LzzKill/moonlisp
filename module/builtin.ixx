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

import moonlisp.value;
import moonlisp.exception;

export namespace moonlisp::runtime::builtin
{
  Value_p print(const std::vector<Value_p>& value, Env_p& env)
  {

    std::visit([&])

    return nullptr;
  }
} // namespace moonlisp::runtime::builtin
