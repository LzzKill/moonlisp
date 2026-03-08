/*
 * @file: constant.ixx
 * @brief: Constants
 * @author LzzKill
 * @license BSD4-Clause License
 *
 *
 * */

module;
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
export module moonlisp.constant;

export namespace moonlisp
{
  constexpr std::string_view VERSION{ "0.1.0" };
  constexpr std::string_view AUTHOR{ "LzzKill" };
  constexpr std::string_view LICENSE{ "BSD-4-Clause" };

  constexpr std::array SYMBOL_TABLE = { '[', ']', '(', ')', '!', '+', '-', '=', '*',
                                        '<', '>', '*', '&', '^', '%', ',', '.', '\'' }; // {}可作为字面量一部分
  constexpr std::array SPACE_TABLE = { ' ', '\f', '\t' };
  constexpr std::array NUMBER_TABLE = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  constexpr std::array NEXT_TABLE = { '\n', '\r' };
  constexpr std::array NOTE_TABLE = { ';', '#' };

  // Define Type
  using Place = std::array<int, 3>;

  // VM ByteCode

  namespace VM
  {
    enum class ByteCode : uint8_t {
      NOP = 0x00,
      POP, // 无参数。弹出栈顶。
      PUSH_CONST, // 参数: const_pool 索引。压入常量 (int, string, nil, lambda等)。
      PUSH_VAR, // 参数: var_name 索引。查找变量并压入。
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

} // namespace moonlisp
