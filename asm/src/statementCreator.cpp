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

#include "statementCreator.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using TokenIterator = std::vector<std::shared_ptr<Token>>::iterator;

static void assureCondition (bool condition, const char* errorMessageTemplate, ...);
static void createStatement (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd);
static bool isExpressionStart (Token::Type tokenType);
static bool isExpressionToken (Token::Type tokenType);

std::vector<std::shared_ptr<Statement>> statementCreator::create (std::vector<std::shared_ptr<Token>>& tokens) {
  std::vector<std::shared_ptr<Statement>> statements;
  statements.reserve (tokens.size () / 3);

  Statement::Builder builder;
  TokenIterator tokenIter = tokens.begin ();
  const TokenIterator tokensEnd = tokens.end ();
  while (tokenIter != tokensEnd) {
    createStatement (builder, tokenIter, tokensEnd);
    statements.push_back (builder.build ());
  }
  return statements;
}

/* The token at tokenIter should be a valid start for an expression.  */
static void addExpression (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  std::shared_ptr<Token> token = *tokenIter;
  const int expressionStartLine = token->line ();
  const int expressionStartColumn = token->column ();
  builder.addToken (token);
  ++tokenIter;

  int parenthesesOpen = token->type () == Token::Type::LEFT_PARENTHESIS;
  int state = parenthesesOpen == 0 ? 1 : 2;

  while (tokenIter != tokensEnd && isExpressionToken ((*tokenIter)->type ())) {
    token = *tokenIter;
    if (state == 1) {
      /* prev token was a RIGHT_PARENTHESIS, an IDENTIFIER or a NUMBER */
      switch (token->type ()) {
      case Token::Type::OPERATOR:
        builder.addToken (token);
        ++tokenIter;
        state = 2;
        break;

      case Token::Type::RIGHT_PARENTHESIS:
        --parenthesesOpen;
        assureCondition (parenthesesOpen > -1, "Error in line %d, column %d: no parenthesis to close.", token->line (), token->column ());
        builder.addToken (token);
        ++tokenIter;
        break;

      case Token::Type::NUMBER:
        printf ("Error in line %d, column %d: missing operator before this number.", token->line (), token->column ());
        exit (EXIT_FAILURE);

      case Token::Type::IDENTIFIER:
        goto expressionTokensCollected;

      case Token::Type::LEFT_PARENTHESIS:
        printf ("Error in line %d, column %d: missing operator before opening parenthesis.", token->line (), token->column ());
        exit (EXIT_FAILURE);
      }
    } else {
      /* prev token was a LEFT_PARENTHESIS or an OPERATOR */
      switch (token->type ()) {
      case Token::Type::OPERATOR:
      case Token::Type::RIGHT_PARENTHESIS:
        printf ("Error in line %d, column %d: a number, identifier or opening parenthesis expected.", token->line (), token->column ());
        exit (EXIT_FAILURE);

      case Token::Type::NUMBER:
      case Token::Type::IDENTIFIER:
        builder.addToken (token);
        ++tokenIter;
        state = 1;
        break;

      case Token::Type::LEFT_PARENTHESIS:
        ++parenthesesOpen;
        builder.addToken (token);
        ++tokenIter;
      }
    }
  }

 expressionTokensCollected:
  assureCondition (state == 1, "The expression starting in line %d, column %d ends with an operator or an opening parenthesis .",
      expressionStartLine, expressionStartColumn);
  assureCondition (parenthesesOpen == 0, "Not all parentheses were closed in the expression starting in line %d, column %d.",
      expressionStartLine, expressionStartColumn);
}

static void assureCondition (bool condition, const char* errorMessageTemplate, ...) {
  if (!condition) {
    va_list argList;
    va_start (argList, errorMessageTemplate);
    vprintf (errorMessageTemplate, argList);
    va_end (argList);
    printf ("\n");
    exit (EXIT_FAILURE);
  }
}

static void assureNotAtEnd (TokenIterator& tokenIter, const TokenIterator& tokensEnd, const char* errorMessage) {
  if (tokenIter == tokensEnd) {
    printf ("Unexpected end of file: %s.\n", errorMessage);
    exit (EXIT_FAILURE);
  }
}

static void createForIdentifier (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "= or : expected after identifier");
  std::shared_ptr<Token> token = *tokenIter;
  ++tokenIter;

  if (token->type () == Token::Type::COLON) {
    builder.type (Statement::Type::LABEL);
    return;
  }
  assureCondition (token->type () == Token::Type::EQUALS, "Error in line %d, column %d: = or : expected after identifier", token->line (), token->column ());
  assureNotAtEnd (tokenIter, tokensEnd, "expression expected");

  token = *tokenIter;
  assureCondition (isExpressionStart (token->type ()), "Error in line %d, column %d: illegal start of expression", token->line (), token->column ());
  builder.type (Statement::Type::CONSTANT);
  addExpression (builder, tokenIter, tokensEnd);
}

static void createStatement (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  std::shared_ptr<Token>& token = *tokenIter;
  builder.addToken (token);
  ++tokenIter;

  switch (token->type ()) {
  case Token::Type::IDENTIFIER:
    createForIdentifier (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::DB:
    builder.type (Statement::Type::DATA);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::SEGREG:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::INSTR0:
    builder.type (Statement::Type::INSTRUCTION);
    break;

  case Token::Type::INSTR01:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::INSTR1:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::INSTR2:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::LOAD:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::IN:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::OUT:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::INT:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::JMP:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  case Token::Type::CONDITIONAL_JMP:
    builder.type (Statement::Type::INSTRUCTION);
    /* TODO: IMPLEMENT */
    break;

  default:
    printf ("Error in line %d, column %d: an instruction identifier, label or constant definition expected.\n", token->line (), token->column ());
    exit (EXIT_FAILURE);
  }
}

static bool isExpressionStart (Token::Type tokenType) {
  return tokenType == Token::Type::NUMBER || tokenType == Token::Type::IDENTIFIER || tokenType == Token::Type::LEFT_PARENTHESIS;
}

static bool isExpressionToken (Token::Type tokenType) {
  return isExpressionStart (tokenType) || tokenType == Token::Type::OPERATOR || tokenType == Token::Type::RIGHT_PARENTHESIS;
}
