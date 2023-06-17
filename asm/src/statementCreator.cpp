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
#include "tokenSubtypes.h"
#include "Operand.h"
#include "exception/ParseException.h"
#include <cstdarg>

#define ACCUMULATOR 0

using TokenIterator = std::vector<std::shared_ptr<Token>>::iterator;

static void assureCondition (bool condition, const char* errorMessageTemplate, ...);
static void assureNotAtEnd (TokenIterator& tokenIter, const TokenIterator& tokensEnd, const char* errorMessage);
static Operand createOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd);
static Operand createPointerOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd);
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

static void addComma (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "comma expected");
  std::shared_ptr<Token>& token = *tokenIter;
  assureCondition (token->type () == Token::Type::COMMA,
      "Error in line %d, column %d: comma expected", token->line (), token->column ());
  builder.addToken (token);
  ++tokenIter;
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
        throw ParseException ("Error in line %d, column %d: missing operator before this number.", token->line (), token->column ());

      case Token::Type::IDENTIFIER:
        goto expressionTokensCollected;

      case Token::Type::LEFT_PARENTHESIS:
        throw ParseException ("Error in line %d, column %d: missing operator before opening parenthesis.", token->line (), token->column ());
      }
    } else {
      /* prev token was a LEFT_PARENTHESIS or an OPERATOR */
      switch (token->type ()) {
      case Token::Type::OPERATOR:
      case Token::Type::RIGHT_PARENTHESIS:
        throw ParseException ("Error in line %d, column %d: a number, identifier or opening parenthesis expected.", token->line (), token->column ());

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

static void addPortOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd, const char* errorMessage) {
  assureNotAtEnd (tokenIter, tokensEnd, errorMessage);
  std::shared_ptr<Token>& token = *tokenIter;
  assureCondition (token->type () == Token::Type::REG && token->subtype () == TOKEN_SUBTYPE_DX || token->type () == Token::Type::NUMBER,
      "Error in line %d, column %d: %s", token->line (), token->column (), errorMessage);

  Operand operand;
  if (token->type () == Token::Type::NUMBER) {
    operand = Operand (Operand::Width::UNDEFINED, Operand::Type::IMMEDIATE, 0);
  } else {
    operand = Operand (Operand::Width::WORD, Operand::Type::REGISTER, TOKEN_SUBTYPE_DX & 0x7);
  }
  builder.addOperand (operand);
  builder.addToken (token);
  ++tokenIter;
}

static void assureCondition (bool condition, const char* errorMessageTemplate, ...) {
  if (!condition) {
    va_list argList;
    va_start (argList, errorMessageTemplate);
    throw ParseException (errorMessageTemplate, argList);
  }
}

static void assureNotAtEnd (TokenIterator& tokenIter, const TokenIterator& tokensEnd, const char* errorMessage) {
  if (tokenIter == tokensEnd) {
    throw ParseException ("Unexpected end of file: %s.", errorMessage);
  }
}

static void createForConditionalJmp (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "label identifier expected");
  std::shared_ptr<Token>& token = *tokenIter;
  assureCondition (token->type () == Token::Type::IDENTIFIER,
      "Error in line %d, column %d: label identifier expected", token->line (), token->column ());
  builder.addToken (token);
  ++tokenIter;
}

static void createForData (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  const char* errorMessage = "a string or a number expected";
  --tokenIter;

  do {
    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, errorMessage);
    std::shared_ptr<Token> token = *tokenIter;
    assureCondition (token->type () == Token::Type::STRING || token->type () == Token::Type::NUMBER,
        "Error in line %d, column %d: %s", token->line (), token->column (), errorMessage);
    builder.addToken (token);
    ++tokenIter;
  } while (tokenIter != tokensEnd && (*tokenIter)->type () == Token::Type::COMMA);
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

static void createForIn (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "al or ax expected");
  std::shared_ptr<Token>& token = *tokenIter;
  Operand operand = createOperand (builder, tokenIter, tokensEnd);
  assureCondition (operand.type () == Operand::Type::REGISTER && operand.id () == ACCUMULATOR,
      "Error in line %d, column %d: al or ax expected", token->line (), token->column ());
  builder.addOperand (operand);

  addComma (builder, tokenIter, tokensEnd);

  addPortOperand (builder, tokenIter, tokensEnd, "a number or dx as second operand expected");
}

static void createForInstructionWithOneOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "operand expected");
  std::shared_ptr<Token> token = *tokenIter;

  Operand::Width width = Operand::Width::UNDEFINED;

  if (token->type () == Token::Type::PTR_TYPE) {
    width = token->subtype () == TOKEN_SUBTYPE_BYTE ? Operand::Width::BYTE : Operand::Width::WORD;

    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, "incomplete operand");
    assureCondition ((*tokenIter)->type () == Token::Type::PTR,
        "Error in line %d, column %d: keyword 'ptr' expected", (*tokenIter)->line (), (*tokenIter)->column ());
    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, "incomplete operand");
    token = *tokenIter;
  }

  Operand operand = createOperand (builder, tokenIter, tokensEnd);
  if (width != Operand::Width::UNDEFINED) {
    assureCondition (operand.type () == Operand::Type::POINTER,
        "Error in line %d, column %d: pointer operand expected after 'ptr' keyword", token->line (), token->column ());
    operand.width (width);
  }
  builder.addOperand (operand);
}

static void createForInstructionWithOneOptionalOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  if (tokenIter != tokensEnd) {
    std::shared_ptr<Token>& token = *tokenIter;
    if (token->type () == Token::Type::NUMBER) {
      builder.addToken (token);
      ++tokenIter;
      builder.addOperand (Operand (Operand::Width::UNDEFINED, Operand::Type::IMMEDIATE, 0));
    }
  }
}

static void createForInstructionWithTwoOperands (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  createForInstructionWithOneOperand (builder, tokenIter, tokensEnd);

  addComma (builder, tokenIter, tokensEnd);

  assureNotAtEnd (tokenIter, tokensEnd, "second operand expected");
  Operand operand = createOperand (builder, tokenIter, tokensEnd);
  builder.addOperand (operand);
}

static void createForInt (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "number expected");
  std::shared_ptr<Token>& token = *tokenIter;
  assureCondition (token->type () == Token::Type::NUMBER,
      "Error in line %d, column %d: number expected", token->line (), token->column ());
  builder.addToken (token);
  ++tokenIter;
}

static void createForJmp (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "a label identifier, the keyword 'far', a pointer or register operand or a number expected");
  std::shared_ptr<Token> token = *tokenIter;
  switch (token->type ()) {
  case Token::Type::IDENTIFIER:
    builder.addToken (token);
    ++tokenIter;
    break;

  case Token::Type::NUMBER:
    builder.addToken (token);
    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, "colon expected");
    token = *tokenIter;
    assureCondition (token->type () == Token::Type::COLON,
        "Error in line %d, column %d: colon expected", token->line (), token->column ());
    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, "number expected");
    token = *tokenIter;
    assureCondition (token->type () == Token::Type::NUMBER,
        "Error in line %d, column %d: number expected", token->line (), token->column ());
    builder.addToken (token);
    ++tokenIter;
    break;

  case Token::Type::FAR:
    builder.addToken (token);
    ++tokenIter;
    assureNotAtEnd (tokenIter, tokensEnd, "a pointer or register operand expected");
    token = *tokenIter;
    assureCondition (token->type () == Token::Type::LEFT_BRACKET || token->type () == Token::Type::REG,
        "Error in line %d, column %d: a pointer or register operand expected", token->line (), token->column ());

  case Token::Type::LEFT_BRACKET:
  case Token::Type::REG: {
    Operand operand = createOperand (builder, tokenIter, tokensEnd);
    builder.addOperand (operand);
  } break;

  default:
    throw ParseException ("Error in line %d, column %d: a label identifier, the keyword 'far', an pointer or register operand or a number expected.",
        token->line (), token->column ());
  }
}

static void createForLoad (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "register operand expected");
  std::shared_ptr<Token> token = *tokenIter;
  Operand operand = createOperand (builder, tokenIter, tokensEnd);
  assureCondition (operand.type () == Operand::Type::REGISTER && operand.width () == Operand::Width::WORD,
      "Error in line %d, column %d: 16 bits register operand expected", token->line (), token->column ());
  builder.addOperand (operand);

  addComma (builder, tokenIter, tokensEnd);

  assureNotAtEnd (tokenIter, tokensEnd, "second operand expected");
  token = *tokenIter;
  operand = createOperand (builder, tokenIter, tokensEnd);
  assureCondition (operand.type () == Operand::Type::POINTER,
      "Error in line %d, column %d: pointer operand expected", token->line (), token->column ());
  operand.width (Operand::Width::WORD);
  builder.addOperand (operand);
}

static void createForOut (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  addPortOperand (builder, tokenIter, tokensEnd, "a number or dx as first operand expected");

  addComma (builder, tokenIter, tokensEnd);

  assureNotAtEnd (tokenIter, tokensEnd, "al or ax expected");
  std::shared_ptr<Token>& token = *tokenIter;
  Operand operand = createOperand (builder, tokenIter, tokensEnd);
  assureCondition (operand.type () == Operand::Type::REGISTER && operand.id () == ACCUMULATOR,
      "Error in line %d, column %d: al or ax expected", token->line (), token->column ());
  builder.addOperand (operand);
}

static void createForSegreg (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  assureNotAtEnd (tokenIter, tokensEnd, "colon expected after segment register");
  std::shared_ptr<Token> token = *tokenIter;
  assureCondition (token->type () == Token::Type::COLON,
      "Error in line %d, column %d: colon expected after segment register", token->line (), token->column ());
  ++tokenIter;
}

static Operand createOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  std::shared_ptr<Token> token = *tokenIter;
  if (isExpressionStart (token->type ())) {
    addExpression (builder, tokenIter, tokensEnd);
    return Operand (Operand::Width::UNDEFINED, Operand::Type::IMMEDIATE, 0);
  } else if (token->type () == Token::Type::REG) {
    builder.addToken (token);
    ++tokenIter;
    Operand::Width width = token->subtype () > 7 ? Operand::Width::WORD : Operand::Width::BYTE;
    return Operand (width, Operand::Type::REGISTER, token->subtype () & 0x7);
  } else if (token->type () == Token::Type::LEFT_BRACKET) {
    return createPointerOperand (builder, tokenIter, tokensEnd);
  } else if (token->type () == Token::Type::SEGREG) {
    builder.addToken (token);
    ++tokenIter;
    return Operand (Operand::Width::WORD, Operand::Type::SEGMENT_REGISTER, token->subtype ());
  } else {
    throw ParseException ("Error in line %d, column %d: operand expected.", token->line (), token->column ());
  }
}

/* The token at tokenIter should be a [  */
static Operand createPointerOperand (Statement::Builder& builder, TokenIterator& tokenIter, const TokenIterator& tokensEnd) {
  std::shared_ptr<Token> token = *tokenIter;
  const int operandStartLine = token->line ();
  const int operandStartColumn = token->column ();
  builder.addToken (token);
  ++tokenIter;

  int code = 0;
  bool plusState = false;
  while (!(tokenIter == tokensEnd || (*tokenIter)->type () == Token::Type::RIGHT_BRACKET)) {
    token = *tokenIter;
    if (plusState) {

      assureCondition ((code & 1) == 0, "Error in line %d, column %d: ] expected", token->line (), token->column ());
      assureCondition (token->type () == Token::Type::OPERATOR && token->subtype () == '+',
          "Error in line %d, column %d: only + can be used between a register and a register or a numerical expression", token->line (), token->column ());
      builder.addToken (token);
      ++tokenIter;

    } else {

      if (token->type () == Token::Type::REG) {
        switch (token->subtype ()) {
        case TOKEN_SUBTYPE_BX:
        case TOKEN_SUBTYPE_BP:
          assureCondition ((code & 0x18) == 0, "Error in line %d, column %d: only one of BX, BP can be used", token->line (), token->column ());
          code |= token->subtype () == TOKEN_SUBTYPE_BX ? 0x8 : 0x10;
          break;

        case TOKEN_SUBTYPE_SI:
        case TOKEN_SUBTYPE_DI:
          assureCondition ((code & 0x6) == 0, "Error in line %d, column %d: only one of SI, DI can be used", token->line (), token->column ());
          code |= token->subtype () == TOKEN_SUBTYPE_SI ? 0x2 : 0x4;
          break;

        default:
          throw ParseException ("Error in line %d, column %d: only the registers BX, BP, SI or DI are allowed to be used here.", token->line (), token->column ());
        }
        builder.addToken (token);
        ++tokenIter;
      } else if (isExpressionStart (token->type ())) {
        addExpression (builder, tokenIter, tokensEnd);
        code |= 1;
      } else {
        throw ParseException ("Error in line %d, column %d: only the registers BX, BP, SI or DI and/or a numerical expression can be used to specify an address.",
            token->line (), token->column ());
      }

    }
    plusState = !plusState;
  }

  assureNotAtEnd (tokenIter, tokensEnd, "incomplete pointer operand");
  assureCondition (code != 0, "Error in line %d, column %d: [] is not a valid operand", operandStartLine, operandStartColumn);
  if (code == 0x10) {
    /* [BP] is not a valid operand, [BP+0] is...  */
    builder.addToken (std::shared_ptr<Token> (new Token (Token::Type::OPERATOR, -2, -2, '+')));
    builder.addToken (std::shared_ptr<Token> (new Token (Token::Type::NUMBER, -2, -2, 0)));
    code = 0x11;
  }
  builder.addToken (*tokenIter);
  ++tokenIter;

  /* :-( */
  int id;
  if (code == 1) {
    id = 6;
  } else {
    switch (code >> 1) {
    case 1: id = 4; break;
    case 2: id = 5; break;
    case 4: id = 7; break;
    case 5: id = 0; break;
    case 6: id = 1; break;
    case 8: id = 6; break;
    case 9: id = 2; break;
    case 10: id = 3;
    }
    if ((code & 1) != 0) {
      id |= 8;
    }
  }
  return Operand (Operand::Width::UNDEFINED, Operand::Type::POINTER, id);
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
    createForData (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::SEGREG:
    builder.type (Statement::Type::INSTRUCTION);
    createForSegreg (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::INSTR0:
    builder.type (Statement::Type::INSTRUCTION);
    break;

  case Token::Type::INSTR01:
    builder.type (Statement::Type::INSTRUCTION);
    createForInstructionWithOneOptionalOperand (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::INSTR1:
    builder.type (Statement::Type::INSTRUCTION);
    createForInstructionWithOneOperand (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::INSTR2:
    builder.type (Statement::Type::INSTRUCTION);
    createForInstructionWithTwoOperands (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::LOAD:
    builder.type (Statement::Type::INSTRUCTION);
    createForLoad (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::IN:
    builder.type (Statement::Type::INSTRUCTION);
    createForIn (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::OUT:
    builder.type (Statement::Type::INSTRUCTION);
    createForOut (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::INT:
    builder.type (Statement::Type::INSTRUCTION);
    createForInt (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::JMP:
    builder.type (Statement::Type::INSTRUCTION);
    createForJmp (builder, tokenIter, tokensEnd);
    break;

  case Token::Type::CONDITIONAL_JMP:
    builder.type (Statement::Type::INSTRUCTION);
    createForConditionalJmp (builder, tokenIter, tokensEnd);
    break;

  default:
    throw ParseException ("Error in line %d, column %d: an instruction identifier, label or constant definition expected.", token->line (), token->column ());
  }
}

static bool isExpressionStart (Token::Type tokenType) {
  return tokenType == Token::Type::NUMBER || tokenType == Token::Type::IDENTIFIER || tokenType == Token::Type::LEFT_PARENTHESIS;
}

static bool isExpressionToken (Token::Type tokenType) {
  return isExpressionStart (tokenType) || tokenType == Token::Type::OPERATOR || tokenType == Token::Type::RIGHT_PARENTHESIS;
}
