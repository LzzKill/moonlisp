/*
 * Code: exception.ixx
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

module;
#include <exception>
#include <format>
#include <iostream>
#include <string>
export module moonlisp.exception;
import moonlisp.constant;

export namespace moonlisp
{
  class MoonlispError : std::exception {
    std::string message;

  public:
    MoonlispError(const std::string &error_s, const Place &p,
                  const std::string &msg) :
      message(
          std::format("{} : {} | Place line: {}, column: {}. All pos: {}",
                      error_s, msg, p[0], p[1], p[2])) { }

    const char *what() const noexcept override { return this->message.c_str(); }
    void show() const { std::cerr << this->message << "\n"; }
  };

  class LexerError final : public MoonlispError {
  public:
    LexerError(const Place &p, const std::string &msg) :
      MoonlispError("LexerError", p, msg) { }
  };

  class ParserError final : public MoonlispError {
  public:
    ParserError(const Place &p, const std::string &msg) :
      MoonlispError("ParserError", p, msg) { }
  };

  class RuntimeError final : public MoonlispError {
  public:
    RuntimeError(const Place &p, const std::string &msg) :
      MoonlispError("RuntimeError", p, msg) { }
  };

  class CompilerError final : public MoonlispError {
  public:
    CompilerError(const Place &p, const std::string &msg) :
      MoonlispError("CompilerError", p, msg) { }
  };
} // namespace moonlisp
