/*
   Author:  Gerard Visser
   e-mail:  visser.gerard(at)gmail.com

   Copyright (C) 2023 Gerard Visser.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "Expression.h"
#include "AddBranch.h"
#include "DivBranch.h"
#include "Leaf.h"
#include "ModBranch.h"
#include "MulBranch.h"
#include "ParenBranch.h"
#include "precedence.h"
#include "SubBranch.h"
#include "../exception/RuntimeException.h"

Expression::Expression (void) : m_parent (nullptr) {
}

Expression::~Expression (void) {
}

Branch* Expression::addBranch (char op) {
  Branch* result;

  switch (op) {
  case '+':
    result = new AddBranch ();
    break;

  case '-':
    result = new SubBranch ();
    break;

  case '*':
    result = new MulBranch ();
    break;

  case '/':
    result = new DivBranch ();
    break;

  case '%':
    result = new ModBranch ();
    break;

  default:
    throw RuntimeException ("%s:%d: illegal operator: %c.", __FILE__, __LINE__, op);
  }

  Expression* expression = this;
  while (expression->m_parent != nullptr && expression->m_parent->precedence () >= result->precedence ()) {
    expression = expression->m_parent;
  }
  if (expression->m_parent != nullptr) {
    expression->m_parent->rightChild (result);
  }
  result->leftChild (expression);
  return result;
}

Branch* Expression::parent (void) {
  return m_parent;
}

static Branch* findParenBranch (Expression* expression) {
  Branch* branch = expression->parent ();
  while (branch != nullptr && branch->precedence () != PAREN_PRECEDENCE) {
    branch = branch->parent ();
  }
  if (branch == nullptr) {
    throw RuntimeException ("%s:%d: no parenthesis to close.", __FILE__, __LINE__);
  }
  return branch;
}

static int64_t findValue (
    const std::string& identifier,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  auto constantsIter = constants.find (identifier);
  if (constantsIter != constants.end ()) {
    return constantsIter->second;
  }
  auto labelsIter = labels.find (identifier);
  if (labelsIter == labels.end ()) {
    throw RuntimeException ("%s:%d: no label or constant with name %s known.", __FILE__, __LINE__, identifier.c_str ());
  }
  return labelsIter->second;
}

std::unique_ptr<const Expression> Expression::create (
    std::shared_ptr<Token>* tokens,
    const int tokenCount,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  std::shared_ptr<Token> token = tokens[0];
  bool operatorExpected;
  Expression* leaf;
  Branch* branch;

  if (token->type () == Token::Type::LEFT_PARENTHESIS) {
    branch = new ParenBranch ();
    operatorExpected = false;
    leaf = nullptr;
  } else if (token->type () == Token::Type::IDENTIFIER) {
    leaf = new Leaf (findValue (token->text (), constants, labels));
    operatorExpected = true;
  } else {
    if (token->type () != Token::Type::NUMBER) {
      throw RuntimeException ("%s:%d: wrong token type.", __FILE__, __LINE__);
    }
    leaf = new Leaf (token->subtype ());
    operatorExpected = true;
  }

  int i = 1;
  while (i < tokenCount && isExpressionToken (tokens[i]->type ())) {
    token = tokens[i];
    if (operatorExpected) {

      if (!(token->type () == Token::Type::OPERATOR || token->type () == Token::Type::RIGHT_PARENTHESIS)) {
        throw RuntimeException ("%s:%d: wrong token type.", __FILE__, __LINE__);
      }
      if (token->type () == Token::Type::OPERATOR) {
        branch = leaf->addBranch (token->subtype ());
      } else {
        leaf = findParenBranch (leaf);
      }
      operatorExpected = token->type () == Token::Type::RIGHT_PARENTHESIS;

    } else {

      if (token->type () == Token::Type::OPERATOR || token->type () == Token::Type::RIGHT_PARENTHESIS) {
        throw RuntimeException ("%s:%d: wrong token type.", __FILE__, __LINE__);
      }
      if (token->type () == Token::Type::IDENTIFIER) {
        leaf = new Leaf (findValue (token->text (), constants, labels));
        branch->rightChild (leaf);
      } else if (token->type () == Token::Type::NUMBER) {
        leaf = new Leaf (token->subtype ());
        branch->rightChild (leaf);
      } else {
        ParenBranch* parenBranch = new ParenBranch ();
        branch->rightChild (parenBranch);
        branch = parenBranch;
      }
      operatorExpected = token->type () != Token::Type::LEFT_PARENTHESIS;

    }
    ++i;
  }

  if (leaf == nullptr) {
    throw RuntimeException ("%s:%d: null pointer exception.", __FILE__, __LINE__);
  }

  Expression* result = leaf;
  while (result->parent () != nullptr) {
    result = result->parent ();
  }
  return std::unique_ptr<const Expression> (result);
}

bool Expression::isExpressionStart (Token::Type tokenType) {
  return tokenType == Token::Type::NUMBER || tokenType == Token::Type::IDENTIFIER || tokenType == Token::Type::LEFT_PARENTHESIS;
}

bool Expression::isExpressionToken (Token::Type tokenType) {
  return isExpressionStart (tokenType) || tokenType == Token::Type::OPERATOR || tokenType == Token::Type::RIGHT_PARENTHESIS;
}
