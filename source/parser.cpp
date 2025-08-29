﻿/*
 * Code: parser.cpp
 *
 * @Author LzzKill
 * @License BSD4-Clause License
 *
 *
 * */

module;
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
module moonlisp.parser;

import moonlisp.lexer;
import moonlisp.exception;

using moonlisp::ast::Atom;
using moonlisp::ast::List;
using moonlisp::ast::Node;
using moonlisp::ast::Pair;

void moonlisp::Parser::parse()
{
  this->getNext(); // 得到第一个 token
  while (this->lex and this->lex->type != _EOF) {
    switch (this->lex->type) {
    case SYMBOL:
      if (this->lex->word == "(")
        this->node.push_back(this->parseList());
      break;
    case _EOF:
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
  this->getNext();
  auto node = std::make_unique<List>();
  while (this->lex->type != _EOF) {
    if (this->isBracket()) { // 只要看是不是 list 或者 pair 即可
      char a = this->lex->word[0];
      switch (a) {
      case ')':
        return Node{std::move(node), this->lex->place};
      case '(': { // 子对象
        node->elements.push_back(this->parseList());
        break;
      }
      case '[':
        node->elements.push_back(this->parsePair());
        break;
      case ']':
        throw ParserError(this->lex->place, "Unmatched ']'");
      }
    } else
      node->elements.push_back(this->parseAtom());
    this->getNext();
  }
  throw ParserError(this->lex->place, "List not closed with ')'");
  return Node{std::move(node), this->lex->place};
}

Node moonlisp::Parser::parsePair()
{ // 只处理 pair
  auto node = std::make_unique<Pair>();
  this->getNext();
  while (this->lex->type != _EOF) { // list or pair or end char?
    if (this->isBracket()) {
      char a = this->lex->word[0];
      switch (a) {
      case ']':
        return Node{std::move(node), this->lex->place};
      case '[': { // 子对象
        node->elements.push_back(this->parsePair());
        break;
      }
      case '(':
        node->elements.push_back(this->parseList());
        break;
      case ')':
        throw ParserError(this->lex->place, "Unmatched ')'");
      }

    } else
      node->elements.push_back(this->parseAtom());
    this->getNext();
  }
  throw ParserError(this->lex->place, "Pair not closed with ']'");
  return Node{std::move(node), this->lex->place};
}

Node moonlisp::Parser::parseAtom()
{ // dot
  if (this->lex->word == ".")
    return Node{std::make_unique<Atom>(Atom{ast::NodeType::DOT,{}}), this->lex->place};
  return Node{(std::make_unique<Atom>(Atom{ast::getNodeType(this->lex->type), std::move(this->lex->word)})),
              this->lex->place};
}

bool moonlisp::Parser::isBracket()
{
  if (this->lex->word.length() != 1) {
    return false;
  }
  auto c = this->lex->word[0];
  switch (c) {
  case '(':
  case ')':
  case '[':
  case ']':
    return true;
  default:
    return false;
  }
}