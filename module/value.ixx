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
  // 字节码指令集
  enum ByteCode : uint8_t {
    NOP = 0x00, // 空操作
    POP, // 弹出栈顶元素
    PUSH_VALUE, // 推送一个值到栈上
    PUSH_VARIABLE, // 推送全局变量到栈上
    PUSH_NIL, // 推送空
    PUSH_LAMBDA, //推送lambda
    MAKE_SYMBOL,
    MAKE_LIST,
    MAKE_PAIR,
    MAKE_MACRO,
    RETURN,
    CALL, // 调用函数
    JUMP, // 无条件跳转
    JUMP_IF_FALSE, // 条件跳转（如果为假）
    HALT // 停止执行
  };

  struct Instruction;


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

  using NativeFunction = std::function<Value_p(std::vector<Value_p> &, Env_p &)>; // 原生函数

  struct Lambda {
    Env_p env;
    std::vector<std::string> params;
    std::vector<moonlisp::Instruction> body_instructions;
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


  // TODO: 直接包含Value_p
  using Operand = std::variant<std::string, double, int, size_t, Lambda>;
  // 指令结构：操作码 + 可选操作数
  struct Instruction {
    ByteCode op;
    std::optional<Operand> operand{ };

    explicit Instruction(const ByteCode opcode) :
      op(opcode), operand(std::nullopt) { }

    explicit Instruction(const ByteCode opcode, const Operand &opnd) :
      op(opcode), operand(opnd) { }
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
    export Value_p make_lambda(Env_p env, std::vector<std::string> params, std::vector<moonlisp::Instruction> body_instructions) {
      return std::make_shared<Value>(Lambda(std::move(env), std::move(params), std::move(body_instructions)));
    }

  } // namespace util
} // namespace moonlisp::runtime
