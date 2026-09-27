/*
 * Code: parser.ixx
 *
 * @Author LzzKill
 * @License BSD4-Clause License
 *
 *
 * */

module;
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>
export module moonlisp.parser;

import moonlisp.ast;
import moonlisp.lexer;
import moonlisp.constant;
import moonlisp.exception;

using moonlisp::ast::Atom;
using moonlisp::ast::List;
using moonlisp::ast::Node;


export namespace moonlisp
{
  class Parser {
    std::unique_ptr<Lexer> lexer;
    ast::TopNode node;
    LexerStruct_p lex;

    void parse();
    void getNext();
    ast::Node parseList(); // 只处理 ()
    ast::Node parseAtom(); // 解析原子

  public:
    explicit Parser(std::unique_ptr<Lexer> lexer) : lexer(std::move(lexer)), lex(nullptr) { this->parse(); }
    ast::TopNode getAST() { return std::move(this->node); }
  };
} // namespace moonlisp

void moonlisp::Parser::parse()
{
  this->getNext(); // 得到第一个 token
  while (this->lex and this->lex->type != moonlisp::LexerType::_EOF) {
    switch (this->lex->type) {
    case moonlisp::LexerType::SYMBOL:
      if (this->lex->word == "(")
        this->node.push_back(this->parseList());
      break;
    case moonlisp::LexerType::_EOF:
      break;
    default:
      throw ParserError(
          this->lex->place,
          std::format("Fields that shouldn't appear: {}", this->lex->word));
    }
    this->getNext();
  }
}

void moonlisp::Parser::getNext()
{
  try {
    this->lex = this->lexer->getNext();
  }
  catch (const LexerError &err) {
    err.show();
  }
}

Node moonlisp::Parser::parseList()
{
  this->getNext(); // skip (
  auto node = std::make_shared<List>();
  while (this->lex->type != moonlisp::LexerType::_EOF) {
      char a = this->lex->word[0];
      switch (a) {
      case ')':
        return Node{std::move(node), this->lex->place};
      case '(': { // 子对象
        node->elements.push_back(this->parseList());
        break;
      }
      //case '[':
      //  node->elements.push_back(this->parsePair());
      //  break;
      //case ']':
      //  throw ParserError(this->lex->place, "Unmatched ']'");
      }
    }
  throw ParserError(this->lex->place, "List not closed with ')'");
  return Node{std::move(node), this->lex->place};
}

//Node moonlisp::Parser::parsePair()
//{ // 只处理 pair
//  auto node = std::make_shared<Pair>();
//  this->getNext();
//  while (this->lex->type != _EOF) { // list or pair or end char?
//    if (this->isBracket()) {
//      char a = this->lex->word[0];
//      switch (a) {
//      case ']':
//        return Node{std::move(node), this->lex->place};
//      case '[': { // 子对象
//        node->elements.push_back(this->parsePair());
//        break;
//      }
//      case '(':
//        node->elements.push_back(this->parseList());
//        break;
//      case ')':
//        throw ParserError(this->lex->place, "Unmatched ')'");
//      }
//
//    } else
//      node->elements.push_back(this->parseAtom());
//    this->getNext();
//  }
//  throw ParserError(this->lex->place, "Pair not closed with ']'");
//  return Node{std::move(node), this->lex->place};
//}

Node moonlisp::Parser::parseAtom()
{ // dot
  //if (this->lex->word == ".") return Node{ std::make_shared<Atom>(Atom{ ast::NodeType::DOT, {} }), this->lex->place };
  return Node(std::make_shared<Atom>(ast::getNodeType(this->lex->type), std::move(this->lex->word)), this->lex->place);
}
