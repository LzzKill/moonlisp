/*
 * Code: lexer.ixx
 *
 * @Author LzzKill
 * @License GNU Public License v3.0
 *
 *
 * */

module;

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

export module moonlisp.lexer;

import moonlisp.constant;
import moonlisp.exception;

export namespace moonlisp
{

   // line column, pos
  
  enum LexerType
  {
    NUMBER,
    FLOAT,
    NAME,
    STRING,
    SYMBOL,
    _EOF
  };

  struct LexerStruct
  {
    LexerType type;
    std::string word;
    Place place;
  };

  using LexerStruct_p = std::unique_ptr<moonlisp::LexerStruct>;

  // TODO: 迭代器
  class Lexer
  {
    std::string input;
    Place place;
    char current;

    public:
    explicit Lexer(std::string input) : input(std::move(input)), place({0,0,0}), current(EOF)
    {
      this->next();
    }
    std::vector<LexerStruct_p> getGroupStruct();
    LexerStruct_p getNext();

    private:
    char next();
    char peek();

    LexerStruct_p makeSymbolLexerStruct();
    template <bool> LexerStruct_p makeNumberLexerStruct();
    LexerStruct_p makeStringLexerStruct();

    inline LexerStruct_p makeLexerStruct(LexerType, std::string &);
  };

  namespace util
  {
    template <typename Table>
    constexpr bool isInTable(const Table &table, char c)
    {
      return std::find(table.begin(), table.end(), c) != table.end();
    }

    export constexpr bool isSymbol(char c)
    {
      return isInTable(SYMBOL_TABLE, c);
    }
    export constexpr bool isWhitespace(char c)
    {
      return isInTable(SPACE_TABLE, c);
    }
    export constexpr bool isNumber(char c)
    {
      return isInTable(NUMBER_TABLE, c);
    }
    export constexpr bool isNextLine(char c)
    {
      return isInTable(NEXT_TABLE, c);
    }
    export constexpr bool isNote(char c) { return isInTable(NOTE_TABLE, c); }
  } // namespace util

} // namespace moonlisp

char moonlisp::Lexer::next()
{
  auto &line = this->place[0];
  auto &column = this->place[1];
  auto &pos = this->place[2];
  if (pos < this->input.length()) {
    this->current = this->input[pos++];
    column++;
    if (this->current == '\r') {
      line++;
      column = 0;
      if (this->peek() == '\n') // CRLF AND CR
        column++;
    }
    if (this->current == '\n') { // ONLY LF
      line++;
      column = 0;
    }
  } else
    this->current = EOF;
  return this->current;
}

char moonlisp::Lexer::peek()
{
  const auto &pos = this->place[2];
  if (pos < this->input.length()) {
    return this->input[pos];
  }
  return EOF;
}

std::vector<std::unique_ptr<moonlisp::LexerStruct>>
moonlisp::Lexer::getGroupStruct()
{
  std::vector<std::unique_ptr<LexerStruct>> result;
  bool e = true;
  while (e) {
    std::unique_ptr<LexerStruct> res = this->getNext();
    if (res->type == _EOF)
      e = false;
    result.emplace_back(std::move(res));
  }
  return result;
}

std::unique_ptr<moonlisp::LexerStruct> moonlisp::Lexer::getNext()
{
  std::string text{};
  while (this->current != EOF) {
    if (util::isNextLine(this->current)) {
      this->next();
      if (!text.empty())
        return this->makeLexerStruct(NAME, text);
      continue;
    }
    if (util::isSymbol(this->current)) { //
      if (!text.empty())
        return this->makeLexerStruct(NAME, text);
      return this->makeSymbolLexerStruct();
    }

    if (util::isNumber(this->current)) {
      if (!text.empty())
        goto _DEFAULT;
      return makeNumberLexerStruct<false>();
    }
    if (util::isWhitespace(this->current)) {
      if (!text.empty())
        return this->makeLexerStruct(NAME, text);
      this->next();
      continue;
    }
    if (util::isNote(this->current)) {
      if (!text.empty())
        return this->makeLexerStruct(NAME, text);
      this->next();
      while (!util::isNextLine(this->current) and this->current != EOF)
        this->next();
      continue;
    }
    if (this->current == '\"') {
      if (!text.empty())
        return this->makeLexerStruct(NAME, text);
      return makeStringLexerStruct();
    }
  _DEFAULT:
    text.push_back(this->current);
    this->next();
  }
  return this->makeLexerStruct(_EOF, text);
}

moonlisp::LexerStruct_p moonlisp::Lexer::makeSymbolLexerStruct()
{
  std::string temp{this->current};
  switch (this->current) {
  case '!': // !=
  case '<': // <=
  case '>': // >=
  case '=': // ==
  {
    if (this->peek() == '=')
      temp.push_back(this->next());
    break;
  }
  case '-': // 负数？
    if (util::isNumber(this->peek())) {
      this->next();
      return this->makeNumberLexerStruct<true>();
    }
  case '+': // ++ 或者 --

    if (this->peek() == this->current)
      temp.push_back(this->next());
    break;
  default: break;
  }
  this->next();
  return this->makeLexerStruct(SYMBOL, temp);
}

template <bool minus>
moonlisp::LexerStruct_p moonlisp::Lexer::makeNumberLexerStruct()
{
  bool isFloat = false;
  std::string result { };
  if constexpr (minus)
    result.push_back('-');
  result.push_back(this->current);

  while (util::isNumber(this->peek()) or
         this->peek() == '.') { // 保证下一个是数字或者点
    if (this->next() == '.') {
      if (isFloat) {
        throw LexerError(this->place, "multiple dots");
        continue;
      }
      isFloat = true;
      result.push_back(this->current);
      continue;
    }
    result.push_back(this->current);
  }
  this->next();

  return this->makeLexerStruct(isFloat ? FLOAT : NUMBER, result);
}

moonlisp::LexerStruct_p moonlisp::Lexer::makeStringLexerStruct()
{
  std::string result;
  while (this->next() != '"' and this->current != EOF) {
    if (this->current == '\\') {
      switch (this->next()) {
      case 'n':
        result.push_back('\n');
        break;
      case 'r':
        result.push_back('\r');
        break;
      case 't':
        result.push_back('\t');
        break;
      case '\\':
        result.push_back('\\');
        break;
      case '\"':
        result.push_back('\"');
        break;
      case '\'':
        result.push_back('\'');
        break;
      default:
        result.push_back(this->current);
        break;
      }
    } else
      result.push_back(this->current);
  }
  if (this->current == EOF) {
    throw LexerError(this->place, "Unclosed string");
  }
  this->next();
  return this->makeLexerStruct(STRING, result);
}

moonlisp::LexerStruct_p moonlisp::Lexer::makeLexerStruct(LexerType type,
                                               std::string &value)
{
  return std::make_unique<LexerStruct>(type, std::move(value), this->place);
}