/*
 * Code: value.ixx
 * @Module moonlisp.runtime: value
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

module;
#include <cstdint>
#include <memory>
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
  enum class ByteCodeVM : uint8_t {
    NOP = 0x00,
    POP, // 无参数。弹出栈顶。
    PUSH_CONST, // 参数: const_pool 索引。压入常量 (int, string, nil, lambda等)。
//  PUSH_NUMBER
    PUSH_VAR, // 参数: var_name 索引。查找变量并压入。
    PUSH_LAMBDA, 
    MAKE_SYMBOL, // 参数: name 索引。创建符号对象压入。
    CONS, // 无参数。弹出 cdr, 弹出 car, 压入 cons(car, cdr)。
    CALL, // 参数: arg_count。弹出 arg_count 个参数 + 1 个函数，执行调用，压入结果。
    RETURN, // 无参数。结束当前帧，将栈顶值作为返回值传递给调用者。
    JUMP, // 参数: 目标指令索引。
    JUMP_IF_FALSE, // 参数: 目标指令索引。若栈顶为假 (#f 或 nil)，则跳转并弹出。
    HALT // 无参数。停止 VM。
    /*
     * NOTE: 虚拟机停机问题：
     * 已知 HALT 命令会让虚拟机停机，那么是否存在一个虚拟机 M(x)
     * 它能够接收任意一段字节码程序 P 和输入数据 I，在有限步骤内判断：
     * 程序 P 在输入 I 上执行时，最终是否会触发 HALT 指令（停机）？
     */
  };
}

export namespace moonlisp
{
  struct Instruction;

  struct Value;
  using Value_p = std::shared_ptr<Value>;


  struct Environment : std::enable_shared_from_this<Environment> {
    public:std::unordered_map<std::string, Value_p> table; // 变量表
    std::shared_ptr<Environment> parent; // 外层作用域
    explicit Environment(std::shared_ptr<Environment> p = nullptr) : parent(std::move(p)) { }


    Value_p get(const std::string &name)
    {
      if (table.contains(name)) return table[name];
      if (parent) return parent->get(name);
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
    std::vector<std::string> params;
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
    ByteCodeVM op;
    Value_p operand{};

    explicit Instruction(const ByteCodeVM opcode) : op(opcode), operand(nullptr) { }

    explicit Instruction(const ByteCodeVM opcode, const Value_p &opnd) : op(opcode), operand(opnd) { }
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
    export auto make_string = make_value<std::string>;
    export auto make_list = make_value<ConsCell>;
    export auto make_native = make_value<NativeFunction>;
    export auto make_symbol = make_value<Symbol>;
    export inline Value_p make_lambda(Env_p env, std::vector<std::string> params, std::vector<Instruction> body)
    {
      Lambda lam{ std::move(env), std::move(params), std::move(body) };
      return make_value(std::move(lam));
    }
  } // namespace util
} // namespace moonlisp
