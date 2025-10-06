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
#include <stack>
export module moonlisp.vm;

import moonlisp.value;
import moonlisp.compiler;
import moonlisp.constant;
import moonlisp.exception;

namespace moonlisp
{
  class VM {
    Instruction_v instructions;
    std::stack<Operand> stack;
    Env_p env;

    void pop();
    void pushValue();
    void pushVariable();

    void pushNil();
    void pushLambda();
    void makeSymbol();
    void makeList();
    void makePair();
    void makeMacro();
    void _return();
    void call();
    void jump();
    void jumpIfFalse();

  public:
    VM(const std::unique_ptr<Compiler> &compiler) :
      instructions(compiler->getInstructions()) { }
    void halt();
    void run();
  };


}
void moonlisp::VM::pop() { }
void moonlisp::VM::pushValue() { }
void moonlisp::VM::pushVariable() { }
void moonlisp::VM::pushNil() { }
void moonlisp::VM::pushLambda() { }
void moonlisp::VM::makeSymbol() { }
void moonlisp::VM::makeList() { }
void moonlisp::VM::makePair() { }
void moonlisp::VM::makeMacro() { }
void moonlisp::VM::_return() { }
void moonlisp::VM::call() { }
void moonlisp::VM::jump() { }
void moonlisp::VM::jumpIfFalse() { }
void moonlisp::VM::halt()
{

}
void moonlisp::VM::run() { }

export moonlisp::VM;
