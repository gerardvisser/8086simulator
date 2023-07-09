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

#include "compiler.h"
#include "parser.h"
#include "tokenSubtypes.h"
#include "exception/ParseException.h"
#include "expressions/Expression.h"
#include <map>

#define UNDEFINED_CONSTANT 0x8000000000000000

static void checkAndAddConstant (
    std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels,
    std::shared_ptr<Statement>& statement);
static void checkForUnknownIdentifiers (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels);
static void checkInstructionAndSetSize (
    std::shared_ptr<Statement>& statement,
    std::vector<std::shared_ptr<Statement>>& jumps,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels);
static int findExpressionStart (std::shared_ptr<Statement>& statement, int operandNo);
static std::map<std::string, int> getLabels (std::vector<std::shared_ptr<Statement>>& statements);
static int getSizeOfInstructionWithPointerOperand (
    std::shared_ptr<Statement>& statement,
    int operandNo,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels);
static void setDataStatementSize (std::shared_ptr<Statement>& statement);
static void setPointerDisplacement (
    std::shared_ptr<Statement>& statement,
    int operandNo,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels);

Compilation compiler::compile (std::istream& stream, int startOffset) {
  std::vector<std::shared_ptr<Statement>> statements = parser::parse (stream);
  std::map<std::string, int> labels = getLabels (statements);
  std::map<std::string, int64_t> constants;
  std::vector<std::shared_ptr<Statement>> jumps;

  int offset = startOffset;
  for (std::shared_ptr<Statement> statement : statements) {
    switch (statement->type ()) {
    case Statement::Type::LABEL:
      labels.find (statement->token (0)->text ())->second = offset;
      break;

    case Statement::Type::CONSTANT:
      checkAndAddConstant (constants, labels, statement);
      break;

    case Statement::Type::INSTRUCTION:
      checkInstructionAndSetSize (statement, jumps, constants, labels);
      break;

    case Statement::Type::DATA:
      setDataStatementSize (statement);
    }
    offset += statement->size ();
  }
  /* TODO */
}

static int calculateExpression (
    std::shared_ptr<Statement>& statement,
    int expressionStartIndex,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  std::shared_ptr<Token>* tokens = statement->tokens () + expressionStartIndex;
  int tokenCount = statement->tokenCount () - expressionStartIndex;
  std::unique_ptr<const Expression> expression = Expression::create (tokens, tokenCount, constants, labels);
  int64_t value = expression->value ();
  if (value < -0x80000000 || value > 0xFFFFFFFF) {
    throw ParseException ("Error in line %d, column %d: expression value too large.", tokens[0]->line (), tokens[0]->column ());
  }
  return value;
}

static bool canCalculateConstant (
    std::shared_ptr<Statement>& statement,
    int expressionStartIndex,
    const std::map<std::string, int64_t>& constants) {
  int i = expressionStartIndex;
  while (i < statement->tokenCount () && Expression::isExpressionToken (statement->token (i)->type ())) {
    std::shared_ptr<Token> token = statement->token (i);
    if (token->type () == Token::Type::IDENTIFIER) {
      auto constantsIter = constants.find (token->text ());
      if (constantsIter == constants.end () || constantsIter->second == UNDEFINED_CONSTANT) {
        return false;
      }
    }
  }
  return true;
}

static void checkAndAddConstant (
    std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels,
    std::shared_ptr<Statement>& statement) {
  std::shared_ptr<Token>& token = statement->token (0);
  const std::string& constant = token->text ();
  auto labelsIter = labels.find (constant);
  if (labelsIter != labels.end ()) {
    throw ParseException ("Error in line %d, column %d: a label with the name %s already exists.", token->line (), token->column (), constant.c_str ());
  }
  auto constantsIter = constants.find (constant);
  if (constantsIter != constants.end ()) {
    throw ParseException ("Error in line %d, column %d: second declaration of constant %s.", token->line (), token->column (), constant.c_str ());
  }
  checkForUnknownIdentifiers (statement, constants, labels);

  int64_t value = UNDEFINED_CONSTANT;
  if (canCalculateConstant (statement, 1, constants)) {
    value = calculateExpression (statement, 1, constants, labels);
  }
  constants[constant] = value;
}

static void checkForUnknownIdentifiers (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  for (int i = 1; i < statement->tokenCount (); ++i) {
    std::shared_ptr<Token>& token = statement->token (i);
    if (token->type () == Token::Type::IDENTIFIER) {
      auto constantsIter = constants.find (token->text ());
      if (constantsIter == constants.end ()) {
        auto labelsIter = labels.find (token->text ());
        if (labelsIter == labels.end ()) {
          throw ParseException ("Error in line %d, column %d: no label or constant with name %s known.", token->line (), token->column (), token->text ().c_str ());
        }
      }
    }
  }
}

static void checkConditionalJmpAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int>& labels) {
  statement->size (2);
  std::shared_ptr<Token>& token = statement->token (1);
  if (labels.find (token->text ()) == labels.end ()) {
    throw ParseException ("Error in line %d, column %d: unknown label %s.", token->line (), token->column (), token->text ().c_str ());
  }
}

static void checkInstructionWithOneOperandAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  std::shared_ptr<Token> token = statement->token (0);
  Operand& operand = statement->operand (0);
  int instrType = token->subtype ();

  if (operand.type () == Operand::Type::SEGMENT_REGISTER && !(instrType == TOKEN_SUBTYPE_PUSH || instrType == TOKEN_SUBTYPE_POP)) {
    throw ParseException ("Error in line %d, column %d: a segment register is not allowed as operand for this instruction.", token->line (), token->column ());
  }
  if (operand.type () == Operand::Type::IMMEDIATE) {
    throw ParseException ("Error in line %d, column %d: this instruction cannot have an immediate operand.", token->line (), token->column ());
  }

  if (instrType == TOKEN_SUBTYPE_PUSH || instrType == TOKEN_SUBTYPE_POP) {
    if (operand.width () == Operand::Width::BYTE) {
      throw ParseException ("Error in line %d, column %d: this instruction can only have 16 bits wide operands.", token->line (), token->column ());
    }
    operand.width (Operand::Width::WORD);
  }
  if (operand.type () == Operand::Type::POINTER) {
    token = statement->token (1);
    if (operand.width () == Operand::Width::UNDEFINED) {
      throw ParseException ("Error in line %d, column %d: operand width unknown.", token->line (), token->column ());
    }
    checkForUnknownIdentifiers (statement, constants, labels);
    statement->size (getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels));
    return;
  }

  if (instrType == TOKEN_SUBTYPE_INC || instrType == TOKEN_SUBTYPE_DEC) {
    statement->size (operand.width () == Operand::Width::WORD ? 1 : 2);
  } else if (instrType == TOKEN_SUBTYPE_PUSH || instrType == TOKEN_SUBTYPE_POP) {
    statement->size (1);
  } else {
    statement->size (2);
  }
}

static void checkJumpAndSetSize (
    std::shared_ptr<Statement>& statement,
    std::vector<std::shared_ptr<Statement>>& jumps,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  if (statement->operandCount () == 1) {
    //opcode=0xFF
    Operand& operand = statement->operand (0);
    if (operand.type () == Operand::Type::POINTER) {
      checkForUnknownIdentifiers (statement, constants, labels);
      statement->size (getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels));
    } else {
      statement->size (2);
    }
  } else if (statement->token (1)->type () == Token::Type::IDENTIFIER) {
    std::shared_ptr<Token>& token = statement->token (1);
    if (labels.find (token->text ()) == labels.end ()) {
      throw ParseException ("Error in line %d, column %d: unknown label %s.", token->line (), token->column (), token->text ().c_str ());
    }
    if (statement->token (0)->subtype () == TOKEN_SUBTYPE_JMP) {
      //opcode=0xEB or 0xE9
      jumps.push_back (statement);
      statement->size (2);
    } else {
      //opcode=0xE8
      statement->size (3);
    }
  } else {
    //opcode=0x9A or 0xEA
    statement->size (5);
  }
}

static void checkInstructionAndSetSize (
    std::shared_ptr<Statement>& statement,
    std::vector<std::shared_ptr<Statement>>& jumps,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  std::shared_ptr<Token> token = statement->token (0);
  switch (token->type ()) {
  case Token::Type::SEGREG:
  case Token::Type::INSTR0:
    //opcode=token->subtype() in case of Token::Type::INSTR0
    statement->size (1);
    break;

  case Token::Type::INSTR01:
    break;

  case Token::Type::INSTR1:
    checkInstructionWithOneOperandAndSetSize (statement, constants, labels);
    break;

  case Token::Type::INSTR2:
    break;

  case Token::Type::LOAD:
    break;

  case Token::Type::IN:
    break;

  case Token::Type::OUT:
    break;

  case Token::Type::INT:
    break;

  case Token::Type::JMP:
    checkJumpAndSetSize (statement, jumps, constants, labels);
    break;

  case Token::Type::CONDITIONAL_JMP:
    checkConditionalJmpAndSetSize (statement, labels);
    break;
  }
}

static int findExpressionStart (std::shared_ptr<Statement>& statement, int operandNo) {
  int i = 1;
  if (operandNo == 1) {
    while (statement->token (i)->type () != Token::Type::COMMA) {
      ++i;
    }
    ++i;
  }
  while (!Expression::isExpressionStart (statement->token (i)->type ())) {
    ++i;
  }
  return i;
}

static std::map<std::string, int> getLabels (std::vector<std::shared_ptr<Statement>>& statements) {
  std::map<std::string, int> labels;
  for (std::shared_ptr<Statement> statement : statements) {
    if (statement->type () == Statement::Type::LABEL) {
      const std::string& label = statement->token (0)->text ();
      auto iter = labels.find (label);
      if (iter != labels.end ()) {
        std::shared_ptr<Token>& token = statement->token (0);
        throw ParseException ("Error in line %d, column %d: second occurrence of label %s.", token->line (), token->column (), label.c_str ());
      }
      labels[label] = -1;
    }
  }
  return labels;
}

/*
   Returns the basic size of an instruction with a pointer operand.  If the instruction also
   has an immediate value then the value returned by this functions needs to be increased.
   An exception is the size of the 'mov acc, [address]' and 'mov [address], acc' instructions.
*/
static int getSizeOfInstructionWithPointerOperand (
    std::shared_ptr<Statement>& statement,
    int operandNo,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  int size;
  Operand& operand = statement->operand (operandNo);
  if (operand.id () > 7) {
    setPointerDisplacement (statement, operandNo, constants, labels);
    if (operand.displacement () < -0x80 || operand.displacement () > 0x7F) {
      size = 4;
    } else {
      size = 3;
    }
  } else if (operand.id () == 6) {
    size = 4;
  } else {
    size = 2;
  }
  return size;
}

static void setDataStatementSize (std::shared_ptr<Statement>& statement) {
  int size = 0;
  for (int i = 1; i < statement->tokenCount (); ++i) {
    std::shared_ptr<Token>& token = statement->token (i);
    if (token->type () == Token::Type::STRING) {
      size += token->text ().length ();
    } else {
      ++size;
    }
  }
  statement->size (size);
}

static void setPointerDisplacement (
    std::shared_ptr<Statement>& statement,
    int operandNo,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  int expressionStart = findExpressionStart (statement, operandNo);
  if (!canCalculateConstant (statement, expressionStart, constants)) {
    std::shared_ptr<Token> token = statement->token (expressionStart);
    throw ParseException ("Error in line %d, column %d: cannot calculate displacement.", token->line (), token->column ());
  }
  int displacement = calculateExpression (statement, expressionStart, constants, labels);
  displacement &= 0xFFFF;
  if ((displacement & 0x8000) != 0) {
    displacement |= 0xFFFF0000;
  }
  statement->operand (operandNo).displacement (displacement);
}
