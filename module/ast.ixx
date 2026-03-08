/**
 * @file ast.ixx
 * @author LzzKill
 * @license GNU Public License v3.0
 */

module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

/**
 * @module moonlisp.ast
 * @brief MoonLisp AST Defined.
 */
export module moonlisp.ast;

import moonlisp.constant;
import moonlisp.lexer;

/**
 * @namespace moonlisp::ast
 */
export namespace moonlisp::ast
{
  /**
   * @enum NodeType
   * @brief 表示 AST 节点的原子类型
   * @details 枚举所有支持的基础数据类型节点，用于区分 Atom 节点的具体值类型
   */
  enum class NodeType: std::int8_t {
    STRING, ///< 字符串类型节点
    NUMBER, ///< 整数类型节点
    FLOAT, ///< 浮点数类型节点
    NAME ///< 标识符/名称类型节点(变量、函数名等)
  };

  struct Atom;
  struct List;

  using Atom_p = std::shared_ptr<Atom>;
  using List_p = std::shared_ptr<List>;

  /**
   * @typedef Node_t
   */
  using Node_t = std::variant<Atom_p, List_p>;

  /**
   * @struct Node
   * @brief AST 节点的通用封装结构
   * @details 包含具体的节点数据(Node_t)和该节点在源代码中的位置信息(Place)
   */
  struct Node {
    Node_t node;
    Place place;
  };

  /**
   * @struct Atom
   * @brief 原子类型 AST 节点
   * @details 表示不可再分的基础数据节点，包含类型标识和字符串形式的数值
   */
  struct Atom {
    NodeType type;
    std::string value; // 运行时转化
  };

  /**
   * @struct List
   * @brief 列表类型 AST 节点
   * @details 表示由多个 AST 节点组成的有序集合，用于描述函数调用、表达式组等复合结构
   */
  struct List {
    std::vector<Node> elements;
  };

  /**
   * @typedef TopNode
   * @brief MoonLisp 顶级 AST 节点类型
   * @details 表示整个源代码文件解析后的 AST 结构，是多个 Node 节点的有序集合
   */
  using TopNode = std::vector<Node>;

  /**
   * @brief 将词法分析器的 Token 类型转换为 AST 节点类型
   * @param type 词法分析器输出的 Token 类型(LexerType)
   * @return 对应的 AST 原子节点类型(NodeType)
   * @details 作为词法分析和语法分析之间的桥梁，将 Token 类型映射为 AST 节点类型，
   *          未匹配的类型默认转换为 NAME 类型(标识符)
   */
  NodeType getNodeType(moonlisp::LexerType type)
  {
    switch (type)
    {
      case FLOAT: return NodeType::FLOAT;
      case NUMBER: return NodeType::NUMBER;
      case STRING: return NodeType::STRING;
      default: return NodeType::NAME;
    }
  };

} // namespace moonlisp::ast
