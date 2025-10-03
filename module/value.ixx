/*
 * Code: value.ixx
 * @Module moonlisp.runtime: value
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

module;
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
export module moonlisp.runtime.value;

import moonlisp.constant;
import moonlisp.ast;

export namespace moonlisp
{
  struct Value;
  // 运行时期的值
  using Value_p = std::shared_ptr<Value>;

  struct Environment : std::enable_shared_from_this<Environment> {
    std::unordered_map<std::string, Value_p> table; // 变量表
    std::shared_ptr<Environment> parent; // 外层作用域
    explicit Environment(std::shared_ptr<Environment> p = nullptr) :
      parent(std::move(p)) { }

    bool exists(const std::string &name) const
    {
      if (table.contains(name)) return true;
      if (!parent) return parent->exists(name);
      return false;
    }

    // if exists, return value; else return nullptr
    Value_p get(const std::string &name)
    {
      if (this->exists(name)) return table[name];
      return nullptr;
    }

    void setLocal(const std::string &name, Value_p v) { table[name] = std::move(v); }

    void setGlobal(const std::string &name, Value_p v)
    {
      if (this->parent) { parent->setGlobal(name, std::move(v)); }
      else { setLocal(name, std::move(v)); }
    };

  };

  using Env_p = std::shared_ptr<Environment>;

  using NativeFunction = std::function<Value_p(const std::vector<Value_p> &, Env_p &)>; // 原生函数

  struct Lambda {
    Env_p env;
    std::vector<std::string> params;
    std::vector<ast::Node> body;
  };

  struct Macro {
    Env_p env;
    std::vector<std::string> params;
    std::vector<ast::Node> body;
  };

  struct ListValue {
    std::vector<Value_p> data;
  };

  struct PairValue {
    std::vector<Value_p> data;
  };

  struct Symbol {
    Value_p data;
  };

  struct Value {
    using Variant = std::variant<std::string, int, double, ListValue, PairValue, Symbol, Lambda, Macro,
                                 NativeFunction>;

    Variant data;

    explicit Value(Variant v) :
      data(std::move(v)) { }
  };

  namespace util
  {
    template<typename T, bool move = true>
    Value_p make_value(T x)
    {
      if constexpr (move) return std::make_shared<Value>(std::move(x));
      else return std::make_shared<Value>(x);
    }

    export auto make_number = make_value<double, false>;
    export auto make_float = make_value<int, false>;
    export auto make_string = make_value<std::string>;
    export auto make_list = make_value<ListValue>;
    export auto make_pair = make_value<PairValue>;
    export auto make_native = make_value<NativeFunction>;
    export auto make_symbol = make_value<Symbol>;
    export auto make_macro = make_value<Macro>;
    export auto make_lambda = make_value<Lambda>;

  } // namespace util
} // namespace moonlisp::runtime
