/**
 * @file ast.ixx
 * @author LzzKill
 * @license GNU Public License v3.0
 */

module;
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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
   * @brief AST 原子节点的类型枚举
   * @details 枚举所有不可再分的基础数据类型节点，用于区分 Atom 节点的具体值类型，
   *          与词法分析器的 LexerType 一一对应（除符号/EOF 外）
   * @var NodeType::STRING 字符串类型原子节点（对应 LexerType::STRING）
   * @var NodeType::NUMBER 整数类型原子节点（对应 LexerType::NUMBER）
   * @var NodeType::FLOAT 浮点数类型原子节点（对应 LexerType::FLOAT）
   * @var NodeType::NAME 标识符类型原子节点（对应 LexerType::NAME/符号/EOF 等）
   */
  enum class NodeType : std::int8_t {
    STRING = 0,
    NUMBER = 1, 
    FLOAT = 2,
    NAME = 3
  };

  // 前向声明
  struct Atom;
  struct List;

  /**
   * @typedef Atom_p
   * @brief 原子节点的共享指针类型
   * @details 使用 shared_ptr 便于 AST 节点的多引用和生命周期管理
   */
  using Atom_p = std::shared_ptr<Atom>;

  /**
   * @typedef List_p
   * @brief 列表节点的共享指针类型
   */
  using List_p = std::shared_ptr<List>;

  /**
   * @typedef Node_t
   * @brief AST 节点的变体类型
   * @details 封装原子节点(Atom_p)和列表节点(List_p)，表示任意类型的 AST 节点
   */
  using Node_t = std::variant<Atom_p, List_p>;

  /**
   * @struct Node
   * @brief AST 节点的通用封装结构
   * @details 包含具体的节点数据（原子/列表）和该节点在源代码中的位置信息，
   *          是所有 AST 节点的统一对外接口
   */
  struct Node {
    Node_t node; ///< 具体的 AST 节点数据（原子/列表变体）
    Place place; ///< 节点在源代码中的位置 [行, 列, 全局偏移量]

    /**
     * @brief 构造函数
     * @param n 具体的 AST 节点数据
     * @param p 节点位置信息
     */
    Node(Node_t n, Place p) : node(std::move(n)), place(std::move(p)) { }
  };

  /**
   * @struct Atom
   * @brief 原子类型 AST 节点
   * @details 表示不可再分的基础数据节点，所有原子节点均以字符串形式存储值（运行时再转换为对应类型），
   *          包含类型标识以区分具体的数据类型
   */
  struct Atom {
    NodeType type; ///< 原子节点的具体类型（字符串/整数/浮点数/标识符）
    std::string value; ///< 原子节点的字符串形式值（统一存储，运行时解析）

    /**
     * @brief 构造函数
     * @param t 原子节点类型
     * @param v 原子节点的值（移动语义）
     */
    Atom(NodeType t, std::string v) : type(t), value(std::move(v)) { }
  };

  /**
   * @struct List
   * @brief 列表类型 AST 节点
   * @details 表示由多个 AST 节点组成的有序集合，用于描述函数调用、表达式组、代码块等复合结构，
   *          是 Lisp 风格语法的核心复合节点
   */
  struct List {
    std::vector<Node> elements;

    /**
     * @brief 构造函数（空列表）
     */
    List() = default;

    /**
     * @brief 构造函数（带初始子节点）
     * @param elems 初始子节点列表（移动语义）
     */
    List(std::vector<Node> elems) : elements(std::move(elems)) { }

    /**
     * @brief 添加子节点到列表末尾
     * @param node 待添加的 AST 节点
     */
    inline void push_back(Node node) { elements.emplace_back(std::move(node)); }
  };

  /**
   * @typedef TopNode
   * @brief MoonLisp 顶级 AST 节点类型
   * @details 表示整个源代码文件解析后的根 AST 结构，是多个通用 Node 节点的有序集合，
   *          对应源代码中的顶级表达式/语句序列
   */
  using TopNode = std::vector<Node>;

  /**
   * @brief 将词法单元类型(LexerType)映射为 AST 原子节点类型(NodeType)
   * @param type 词法分析器输出的 Token 类型
   * @return 对应的 AST 原子节点类型
   * @details 作为词法分析和语法分析的桥梁，处理所有 LexerType 类型：
   *          - FLOAT/NUMBER/STRING 直接映射为对应 NodeType
   *          - 其他类型（NAME/SYMBOL/_EOF）统一映射为 NAME 类型
   */
  inline NodeType getNodeType(moonlisp::LexerType type)
  {
    switch (type)
    {
      case moonlisp::LexerType::FLOAT: return NodeType::FLOAT;
      case moonlisp::LexerType::NUMBER: return NodeType::NUMBER;
      case moonlisp::LexerType::STRING: return NodeType::STRING;
      case moonlisp::LexerType::NAME:
      case moonlisp::LexerType::SYMBOL:
      case moonlisp::LexerType::_EOF:
      default: return NodeType::NAME;
    }
  }

} // namespace moonlisp::ast
