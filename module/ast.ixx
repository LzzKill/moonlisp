/*
 * Code: ast.ixx
 *
 * @Author LzzKill
 * @License BSD4-Clause License
 *
 *
 * */

module;
#include <memory>
#include <string>
#include <variant>
#include <vector>

export module moonlisp.ast;

import moonlisp.constant;
import moonlisp.lexer;

export namespace moonlisp::ast
{
  enum class NodeType
  {
    STRING,
    NUMBER,
    FLOAT,
    NAME, // IDENT
    DOT   // .
  };

  struct Atom;
  struct List;
  struct Pair;

  using Atom_p = std::shared_ptr<Atom>;
  using List_p = std::shared_ptr<List>;
  using Pair_p = std::shared_ptr<Pair>;

  using Node_t = std::variant<Atom_p, List_p, Pair_p>;

  struct Node
  {
    Node_t node;
    Place place; // 位置信息
  };

  struct Atom
  {
    NodeType type;
    std::string value;
  };

  struct List
  {
    std::vector<Node> elements;
  };

  struct Pair
  {
    std::vector<Node> elements;
  };

  using TopNode = std::vector<Node>;

  // 辅助函数

  NodeType getNodeType(moonlisp::LexerType type)
  {
    switch (type) {
    case FLOAT:
      return NodeType::FLOAT;
    case NUMBER:
      return NodeType::NUMBER;
    case STRING:
      return NodeType::STRING;
    default:
      return NodeType::NAME;
    }
  };
} // namespace moonlisp::ast
