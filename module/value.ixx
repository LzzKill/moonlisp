/*
 * Code: value.ixx
 * @Module moonlisp.runtime: value
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

module;
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
export module moonlisp.value;

import moonlisp.constant;
import moonlisp.ast;


export namespace moonlisp
{
  struct Instruction;

  struct Value;
  using Value_p = std::shared_ptr<Value>;


  struct Environment : std::enable_shared_from_this<Environment> {
    std::unordered_map<std::string, Value_p> table; // 变量表
    std::shared_ptr<Environment> parent; // 外层作用域
    explicit Environment(std::shared_ptr<Environment> p = nullptr) : parent(std::move(p)) { }

    bool exists(const std::string &name) const
    {
      if (table.contains(name)) return true;
      if (parent) return parent->exists(name);
      return false;
    }
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

  /*
  * @typedef NativeFunction
  * @brief 原生函数
  * */
  using NativeFunction = Value_p (*)(std::vector<Value_p> &, Env_p &); // 使用函数指针

  struct Lambda {
    Env_p env;
    std::vector<std::string> prams;
    std::vector<Instruction> body_instructions;
  };

  struct ConsCell {
    Value_p car;
    Value_p cdr;

    ConsCell(Value_p car, Value_p cdr) : car(std::move(car)), cdr(std::move(cdr)) { }
  };

  struct Symbol {
    Value_p data;
  };

  struct Value {
    using Variant = std::variant<std::string, int, double, ConsCell, Symbol, Lambda, NativeFunction>;

    Variant data;

    explicit Value(Variant v) : data(std::move(v)) { }
  };

  // 指令结构：操作码 + 可选操作数
  struct Instruction {
    VM::ByteCode op;
    Value_p operand{ };

    explicit Instruction(const VM::ByteCode opcode) :
      op(opcode), operand(nullptr) { }

    explicit Instruction(const VM::ByteCode opcode, const Operand &opnd) :
      op(opcode), operand(opnd) { }
  };

  namespace util
  {
    template<typename T, bool move = true>
    Value_p make_value(T x)
    {
      if constexpr (move)
        return std::make_shared<Value>(std::move(x));
      else
        return std::make_shared<Value>(x);
    }

    export auto make_number = make_value<int, false>;
    export auto make_float = make_value<double, false>;
    // export auto make_string = make_value<std::string>;
    export auto make_list = make_value<ConsCell>;
    export auto make_native = make_value<NativeFunction>;
    export auto make_symbol = make_value<Symbol>;

  } // namespace util
} // namespace moonlisp
