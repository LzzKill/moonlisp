/*
 * Code: builtin.ixx
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 * 虚拟机实现
 * */

module;
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>

export module moonlisp.vm;

import moonlisp.value;
import moonlisp.compiler;
import moonlisp.constant;
import moonlisp.exception;
import moonlisp.builtin;

namespace moonlisp
{
  // VM ByteCode

  class VM {
    Instruction_v instructions;
    std::stack<Value_p> stack;
    Env_p env_root, env_current;

    void pop();
    void pushValue(const Instruction &m);
    void pushVariable(const Instruction &m);
    void pushNil();
    void pushLambda(const Instruction &m);
    void makeSymbol(const Instruction &m);
    void makeList(const Instruction &m);
    void makePair(const Instruction &m);
    void makeMacro(const Instruction &m);
    void ret();
    void call(const Instruction &m);
    void jump(const Instruction &m);
    void jumpIfFalse(const Instruction &m);
    // 对于 add 等原子操作，使用 builtins 实现

  public:
    VM(const std::unique_ptr<Compiler> &compiler) : instructions(compiler->getInstructions()) { }

    void run();
  };


} // namespace moonlisp

void moonlisp::VM::pop() { }
void moonlisp::VM::pushValue(const Instruction &m) { }
void moonlisp::VM::pushVariable(const Instruction &m) { }
void moonlisp::VM::pushNil() { }
void moonlisp::VM::pushLambda(const Instruction &m) { }
void moonlisp::VM::makeSymbol(const Instruction &m) { }
void moonlisp::VM::makeList(const Instruction &m) { }
void moonlisp::VM::makePair(const Instruction &m) { }
void moonlisp::VM::makeMacro(const Instruction &m) { }
void moonlisp::VM::ret() { }
void moonlisp::VM::call(const Instruction &m) { }
void moonlisp::VM::jump(const Instruction &m) { }
void moonlisp::VM::jumpIfFalse(const Instruction &m) { }

void moonlisp::VM::run()
{
  this->env = getBuiltin(); // 直接用 builtins得到
  for (const auto &m : this->instructions)
  {
    switch (m.op)
    {
      case NOP: break;
      case POP: break;
      case PUSH_VALUE: {
        this->pushValue(m);
        break;
      }
      case PUSH_VARIABLE: break;
      case PUSH_NIL: break;
      case PUSH_LAMBDA: break;
      case MAKE_LIST: break;
      case MAKE_PAIR: break;
      case MAKE_MACRO: break;
      case RETURN: break;
      case CALL: break;
      case JUMP: break;
      case JUMP_IF_FALSE: break;
      case HALT: throw Exit(0);
      case PUSH_SYMBOL: break;
    }
  }
}

export moonlisp::VM;
