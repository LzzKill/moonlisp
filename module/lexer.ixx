/**
 * @file lexer.ixx
 * @author LzzKill
 * @license GNU Public License v3.0
 */

module;

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

export module moonlisp.lexer;

import moonlisp.constant;
import moonlisp.exception;

/**
 * @namespace moonlisp::util
 */
namespace moonlisp::util
{
  template<typename Table>
  constexpr bool isInTable(const Table &table, char c)
  {
    return std::find(table.begin(), table.end(), c) != table.end();
  }
  export constexpr bool isSymbol(char c) { return isInTable(SYMBOL_TABLE, c); }
  export constexpr bool isWhitespace(char c) { return isInTable(SPACE_TABLE, c); }
  export constexpr bool isNumber(char c) { return isInTable(NUMBER_TABLE, c); }
  export constexpr bool isNextLine(char c) { return isInTable(NEXT_TABLE, c); }
  export constexpr bool isNote(char c) { return isInTable(NOTE_TABLE, c); }
} // namespace moonlisp::util

/**
 * @namespace moonlisp
 */
export namespace moonlisp
{
  /**
   * @enum LexerType
   * @brief 词法单元类型枚举
   * @var LexerType::NUMBER 整数类型词法单元
   * @var LexerType::FLOAT 浮点数类型词法单元
   * @var LexerType::NAME 标识符/名称类型词法单元
   * @var LexerType::STRING 字符串类型词法单元
   * @var LexerType::SYMBOL 符号（运算符/分隔符）类型词法单元
   * @var LexerType::_EOF 输入结束标记
   */
  enum class LexerType { NUMBER, FLOAT, NAME, STRING, SYMBOL, _EOF };

  /**
   * @struct LexerStruct
   * @brief 词法单元结构体，存储词法分析结果
   * @details 包含词法单元类型、文本内容、源码位置（行/列/全局偏移）
   */
  struct LexerStruct {
    LexerType type; ///< 词法单元类型
    std::string word; ///< 词法单元文本内容
    Place place; ///< 源码位置

    /**
     * @brief 构造函数
     * @param t 词法单元类型
     * @param w 文本内容（移动语义）
     * @param p 源码位置
     */
    LexerStruct(LexerType t, std::string w, Place p) : type(t), word(std::move(w)), place(std::move(p)) { }
  };

  /**
   * @typedef LexerStruct_p
   */
  using LexerStruct_p = std::unique_ptr<LexerStruct>;

  /**
   * @class Lexer
   */
  class Lexer {

    std::string input; ///< 待解析的输入字符串
    Place place; ///< 当前解析位置 [行(0开始), 列(0开始), 全局偏移(0开始)]
    char current; ///< 当前正在处理的字符

    /**
     * @brief 读取下一个字符并更新位置信息
     * @return 读取到的字符，EOF 表示输入结束
     */
    inline char next()
    {
      int &line = place[0];
      int &column = place[1];
      int &pos = place[2];

      if (pos < input.length())
      {
        current = input[pos++];
        column++;

        // 统一处理换行符（CR、LF、CRLF）
        if (current == '\r')
        {
          line++;
          column = 0;
          // 处理 CRLF 组合（跳过 LF）
          if (pos < input.length() && input[pos] == '\n')
          {
            pos++;
            column++;
          }
        }
        else if (current == '\n')
        {
          line++;
          column = 0;
        }
      }
      else { current = EOF; }
      return current;
    }

    /**
     * @brief 查看下一个字符（不移动解析指针）
     * @return 下一个字符，EOF 表示输入结束
     */
    inline char peek()
    {
      int pos = place[2]; // 改为值拷贝，避免无意义引用
      return (pos < input.length()) ? input[pos] : EOF;
    }

    /**
     * @brief 构造符号类型词法单元（+、-、!= 等）
     * @return 符号类型词法单元的唯一指针
     */
    inline LexerStruct_p makeSymbolLexerStruct()
    {
      std::string temp{ current };
      switch (current)
      {
        case '!': // !=
        case '<': // <=
        case '>': // >=
        case '=': // ==
          if (peek() == '=') temp.push_back(next());
          break;
        case '-': // 处理负数（-123）或普通减号/--
          if (util::isNumber(peek()))
          {
            next();
            return makeNumberLexerStruct<true>();
          }
          [[fallthrough]]; // 非负数则走 + 的逻辑
        case '+': // ++ 或普通加号
          if (peek() == current) temp.push_back(next());
          break;
        default: break;
      }
      next();
      return makeLexerStruct(LexerType::SYMBOL, temp);
    }

    /**
     * @brief 构造数字类型词法单元（整数/浮点数）
     * @tparam minus 是否为负数（true: 负数，false: 非负数）
     * @return 数字类型词法单元的唯一指针
     * @throw LexerError 浮点数包含多个小数点时抛出
     */
    template<bool minus>
    inline LexerStruct_p makeNumberLexerStruct()
    {
      bool isFloat = false;
      std::string result;

      if constexpr (minus) result.push_back('-');
      result.push_back(current);

      // 读取数字/小数点
      while (util::isNumber(peek()) || peek() == '.')
      {
        next();
        if (current == '.')
        {
          if (isFloat) throw LexerError(place, "multiple dots in float number");
          isFloat = true;
        }
        result.push_back(current);
      }
      next();

      return makeLexerStruct(isFloat ? LexerType::FLOAT : LexerType::NUMBER, result);
    }

    /**
     * @brief 构造字符串类型词法单元（处理转义字符）
     * @return 字符串类型词法单元的唯一指针
     * @throw LexerError 字符串未闭合时抛出
     */
    inline LexerStruct_p makeStringLexerStruct()
    {
      std::string result;
      // 跳过开头的 "，读取到结尾的 " 或 EOF
      while (next() != '"' && current != EOF)
      {
        if (current == '\\') // 处理转义字符
        {
          switch (next())
          {
            case 'n': result.push_back('\n'); break;
            case 'r': result.push_back('\r'); break;
            case 't': result.push_back('\t'); break;
            case '\\': result.push_back('\\'); break;
            case '"': result.push_back('"'); break;
            case '\'': result.push_back('\''); break;
            default: result.push_back(current); break;
          }
        }
        else { result.push_back(current); }
      }

      if (current == EOF) throw LexerError(place, "Unclosed string literal");

      next(); // 跳过结尾的 "
      return makeLexerStruct(LexerType::STRING, result);
    }

    /**
     * @brief 构造词法单元的通用辅助函数
     * @param type 词法单元类型
     * @param value 文本内容（移动语义）
     * @return 词法单元的唯一指针
     */
    inline LexerStruct_p makeLexerStruct(LexerType type, std::string &value)
    {
      return std::make_unique<LexerStruct>(type, std::move(value), place);
    }

  public:
    /**
     * @brief 构造函数，初始化词法分析器
     * @param input 待解析的输入字符串
     */
    explicit Lexer(std::string input) : input(std::move(input)), place({ 0, 0, 0 }), current(EOF)
    {
      next(); // 读取第一个字符
    }

    /**
     * @brief 获取下一个词法单元
     * @return 下一个词法单元的唯一指针（_EOF 表示解析结束）
     */
    inline LexerStruct_p getNext()
    {
      std::string text;
      while (current != EOF)
      {
        // 处理换行符
        if (util::isNextLine(current))
        {
          next();
          if (!text.empty()) return makeLexerStruct(LexerType::NAME, text);
          continue;
        }

        // 处理符号
        if (util::isSymbol(current))
        {
          if (!text.empty()) return makeLexerStruct(LexerType::NAME, text);
          return makeSymbolLexerStruct();
        }

        // 处理数字
        if (util::isNumber(current))
        {
          if (!text.empty())
          {
            text.push_back(current); // 替代 goto，优化逻辑
            next();
            continue;
          }
          return makeNumberLexerStruct<false>();
        }

        // 处理空白符
        if (util::isWhitespace(current))
        {
          if (!text.empty()) return makeLexerStruct(LexerType::NAME, text);
          next();
          continue;
        }

        // 处理注释
        if (util::isNote(current))
        {
          if (!text.empty()) return makeLexerStruct(LexerType::NAME, text);
          next();
          // 跳过注释直到换行/EOF
          while (!util::isNextLine(current) && current != EOF) next();
          continue;
        }

        // 处理字符串
        if (current == '"')
        {
          if (!text.empty()) return makeLexerStruct(LexerType::NAME, text);
          return makeStringLexerStruct();
        }

        // 普通字符（标识符）
        text.push_back(current);
        next();
      }

      // 解析结束，返回 EOF 标记
      return makeLexerStruct(LexerType::_EOF, text);
    }
  };


} // namespace moonlisp

template moonlisp::LexerStruct_p moonlisp::Lexer::makeNumberLexerStruct<true>();
template moonlisp::LexerStruct_p moonlisp::Lexer::makeNumberLexerStruct<false>();
