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

#define ACCUMULATOR 0
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
static int determineSizeOfJumpInstructions (
    std::vector<std::shared_ptr<Statement>>& statements,
    std::vector<std::shared_ptr<Statement>>& jumps,
    std::map<std::string, int>& labels,
    int startOffset);
static int findExpressionStart (std::shared_ptr<Statement>& statement, int operandNo);
static int findTokenIndexOfOperand (std::shared_ptr<Statement>& statement, int operandNo);
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
static void setWidthOfImmediateOperandOfAddInstruction (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels);

Compilation compiler::compile (std::istream& stream, int startOffset) {
  std::vector<std::shared_ptr<Statement>> statements = parser::parse (stream);
  if (statements.empty ()) {
    return Compilation (0);
  }

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
    statement->location (offset);
    offset += statement->size ();
  }
  offset += determineSizeOfJumpInstructions (statements, jumps, labels, startOffset);

  Compilation result (offset - startOffset);
  if (result.size () > 0) {
    /* TODO */
  }
  return result;
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

static int calculateWordValue (
    std::shared_ptr<Statement>& statement,
    int expressionStartIndex,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  int value = calculateExpression (statement, expressionStartIndex, constants, labels);
  value &= 0xFFFF;
  if ((value & 0x8000) != 0) {
    value |= 0xFFFF0000;
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

static void checkInAndSetSize (std::shared_ptr<Statement>& statement) {
  Operand& srcOperand = statement->operand (1);
  if (srcOperand.type () == Operand::Type::REGISTER) {
    statement->size (1);
  } else {
    std::shared_ptr<Token>& token = statement->token (3);
    int port = token->subtype ();
    if (port <= -1 || port > 0xFF) {
      throw ParseException ("Error in line %d, column %d: the port value should be in the range [0, 0xFF].", token->line (), token->column ());
    }
    srcOperand.width (Operand::Width::BYTE);
    statement->size (2);
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

static void checkInstructionWithOneOptionalOperandAndSetSize (std::shared_ptr<Statement>& statement) {
  std::shared_ptr<Token> token = statement->token (0);
  if (token->subtype () == TOKEN_SUBTYPE_RET || token->subtype () == TOKEN_SUBTYPE_RETF) {
    if (statement->tokenCount () == 2) {
      token = statement->token (1);
      int number = token->subtype ();
      if (number <= -0x8001 || number > 0xFFFF) {
        throw ParseException ("Error in line %d, column %d: the number value should be in the range [-0x8000, 0xFFFF].", token->line (), token->column ());
      }
      statement->operand (0).width (Operand::Width::WORD);
      statement->size (3);
    } else {
      statement->size (1);
    }
  } else {
    /* AAM and AAD */
    if (statement->tokenCount () == 2) {
      token = statement->token (1);
      int number = token->subtype ();
      if (number <= 0 || number > 0xFF) {
        throw ParseException ("Error in line %d, column %d: the number value should be in the range [1, 0xFF].", token->line (), token->column ());
      }
      statement->operand (0).width (Operand::Width::BYTE);
    }
    statement->size (2);
  }
}

static void checkOperandWidthsAreTheSame(std::shared_ptr<Statement>& statement) {
  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);
  if (dstOperand.width () != srcOperand.width ()) {
    std::shared_ptr<Token> token = statement->token (0);
    throw ParseException ("Error in line %d, column %d: operands of instruction are of different size.", token->line (), token->column ());
  }
}

static void checkAddAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);

  if (dstOperand.type () == Operand::Type::REGISTER) {
    if (srcOperand.type () == Operand::Type::REGISTER) {
      checkOperandWidthsAreTheSame (statement);
      statement->size (2);
    } else if (srcOperand.type () == Operand::Type::POINTER) {
      srcOperand.width (dstOperand.width ());
      statement->size (getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels));
    } else {
      if (dstOperand.id () == ACCUMULATOR) {
        srcOperand.width (dstOperand.width ());
        statement->size (dstOperand.width () == Operand::Width::BYTE ? 2 : 3);
      } else {
        setWidthOfImmediateOperandOfAddInstruction (statement, constants, labels);
        statement->size (srcOperand.width () == Operand::Width::BYTE ? 3 : 4);
      }
    }
  } else {
    /* dstOperand.type () == Operand::Type::POINTER */
    int size = getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels);
    if (srcOperand.type () == Operand::Type::REGISTER) {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        dstOperand.width (srcOperand.width ());
      } else {
        checkOperandWidthsAreTheSame (statement);
      }
    } else {
      /* srcOperand.type () == Operand::Type::IMMEDIATE */
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        std::shared_ptr<Token> token = statement->token (1);
        throw ParseException ("Error in line %d, column %d: operand size unknown.", token->line (), token->column ());
      }
      setWidthOfImmediateOperandOfAddInstruction (statement, constants, labels);
      size += srcOperand.width () == Operand::Width::BYTE ? 1 : 2;
    }
    statement->size (size);
  }
}

static void checkMovAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);

  if (dstOperand.type () == Operand::Type::REGISTER) {
    if (srcOperand.type () == Operand::Type::REGISTER || srcOperand.type () == Operand::Type::SEGMENT_REGISTER) {
      checkOperandWidthsAreTheSame (statement);
      statement->size (2);
    } else if (srcOperand.type () == Operand::Type::POINTER) {
      srcOperand.width (dstOperand.width ());
      if (dstOperand.id () == ACCUMULATOR && srcOperand.id () == 6) {
        statement->size (3);
      } else {
        statement->size (getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels));
      }
    } else {
      srcOperand.width (dstOperand.width ());
      statement->size (dstOperand.width () == Operand::Width::BYTE ? 2 : 3);
    }
  } else if (dstOperand.type () == Operand::Type::POINTER) {
    int size = getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels);
    if (srcOperand.type () == Operand::Type::SEGMENT_REGISTER) {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        dstOperand.width (srcOperand.width ());
      } else {
        checkOperandWidthsAreTheSame (statement);
      }
    } else if (srcOperand.type () == Operand::Type::REGISTER) {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        dstOperand.width (srcOperand.width ());
      } else {
        checkOperandWidthsAreTheSame (statement);
      }
      if (dstOperand.id () == 6 && srcOperand.id () == ACCUMULATOR) {
        size = 3;
      }
    } else {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        std::shared_ptr<Token> token = statement->token (1);
        throw ParseException ("Error in line %d, column %d: operand size unknown.", token->line (), token->column ());
      }
      srcOperand.width (dstOperand.width ());
      size += srcOperand.width () == Operand::Width::BYTE ? 1 : 2;
    }
    statement->size (size);
  } else {
    /* dstOperand.type () == Operand::Type::SEGMENT_REGISTER */
    if (srcOperand.type () == Operand::Type::SEGMENT_REGISTER) {
      std::shared_ptr<Token> token = statement->token (0);
      throw ParseException ("Error in line %d, column %d: cannot directly move the value of one segment register into another.", token->line (), token->column ());
    } else if (srcOperand.type () == Operand::Type::REGISTER) {
      checkOperandWidthsAreTheSame (statement);
      statement->size (2);
    } else if (srcOperand.type () == Operand::Type::POINTER) {
      srcOperand.width (Operand::Width::WORD);
      int size = getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels);
      statement->size (size);
    } else {
      std::shared_ptr<Token> token = statement->token (findTokenIndexOfOperand (statement, 1));
      throw ParseException ("Error in line %d, column %d: cannot directly set the value of a segment register (with an immediate).", token->line (), token->column ());
    }
  }
}

static void checkShiftAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);

  if (srcOperand.type () == Operand::Type::REGISTER && !(srcOperand.width () == Operand::Width::BYTE && srcOperand.id () == 1)
      || srcOperand.type () == Operand::Type::POINTER) {
    std::shared_ptr<Token> token = statement->token (findTokenIndexOfOperand (statement, 1));
    throw ParseException ("Error in line %d, column %d: only CL or an immediate value allowed as source operand.", token->line (), token->column ());
  }
  int immValue;
  if (srcOperand.type () == Operand::Type::IMMEDIATE) {
    int expressionStart = findExpressionStart (statement, 1);
    if (!canCalculateConstant (statement, expressionStart, constants)) {
      std::shared_ptr<Token> token = statement->token (expressionStart);
      throw ParseException ("Error in line %d, column %d: the value to shift should be a constant.", token->line (), token->column ());
    }
    immValue = calculateWordValue (statement, expressionStart, constants, labels);
    if (immValue <= -1 || immValue > 0xFF) {
      std::shared_ptr<Token> token = statement->token (expressionStart);
      throw ParseException ("Error in line %d, column %d: the value to shift should be greater than 0 but less than 256.", token->line (), token->column ());
    }
  }

  if (dstOperand.type () == Operand::Type::REGISTER) {
    if (srcOperand.type () == Operand::Type::REGISTER) {
      statement->size (2);
    } else {
      statement->size (immValue == 1 ? 2 : 3);
    }
  } else {
    /* dstOperand.type () == Operand::Type::POINTER */
    if (dstOperand.width () == Operand::Width::UNDEFINED) {
      std::shared_ptr<Token> token = statement->token (1);
      throw ParseException ("Error in line %d, column %d: operand size unknown.", token->line (), token->column ());
    }
    int size = getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels);
    if (srcOperand.type () == Operand::Type::IMMEDIATE) {
      size += immValue != 1;
    }
    statement->size (size);
  }
}

static void checkTestAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);

  if (dstOperand.type () == Operand::Type::REGISTER) {
    if (srcOperand.type () == Operand::Type::REGISTER) {
      checkOperandWidthsAreTheSame (statement);
      statement->size (2);
    } else if (srcOperand.type () == Operand::Type::POINTER) {
      srcOperand.width (dstOperand.width ());
      statement->size (getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels));
    } else {
      srcOperand.width (dstOperand.width ());
      if (dstOperand.id () == ACCUMULATOR) {
        statement->size (srcOperand.width () == Operand::Width::BYTE ? 2 : 3);
      } else {
        statement->size (srcOperand.width () == Operand::Width::BYTE ? 3 : 4);
      }
    }
  } else if (dstOperand.type () == Operand::Type::POINTER) {
    int size = getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels);
    if (srcOperand.type () == Operand::Type::REGISTER) {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        dstOperand.width (srcOperand.width ());
      } else {
        checkOperandWidthsAreTheSame (statement);
      }
    } else {
      if (dstOperand.width () == Operand::Width::UNDEFINED) {
        std::shared_ptr<Token> token = statement->token (1);
        throw ParseException ("Error in line %d, column %d: operand size unknown.", token->line (), token->column ());
      }
      srcOperand.width (dstOperand.width ());
      size += srcOperand.width () == Operand::Width::BYTE ? 1 : 2;
    }
    statement->size (size);
  } else {
    /* dstOperand.type () == Operand::Type::IMMEDIATE */
    if (srcOperand.type () == Operand::Type::REGISTER) {
      dstOperand.width (srcOperand.width ());
      if (srcOperand.id () == ACCUMULATOR) {
        statement->size (srcOperand.width () == Operand::Width::BYTE ? 2 : 3);
      } else {
        statement->size (srcOperand.width () == Operand::Width::BYTE ? 3 : 4);
      }
    } else if (srcOperand.type () == Operand::Type::POINTER) {
      std::shared_ptr<Token> token = statement->token (findTokenIndexOfOperand (statement, 1));
      throw ParseException ("Error in line %d, column %d: operand size unknown; switch the operands.", token->line (), token->column ());
    } else {
      std::shared_ptr<Token> token = statement->token (findTokenIndexOfOperand (statement, 1));
      throw ParseException ("Error in line %d, column %d: the test instruction can't have two immediate operands.", token->line (), token->column ());
    }
  }
}

static void checkXchgAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);

  if (srcOperand.type () == Operand::Type::IMMEDIATE) {
    std::shared_ptr<Token> token = statement->token (findTokenIndexOfOperand (statement, 1));
    throw ParseException ("Error in line %d, column %d: the xchg instruction can't have immediate operands.", token->line (), token->column ());
  }

  if (dstOperand.type () == Operand::Type::REGISTER) {
    if (srcOperand.type () == Operand::Type::REGISTER) {
      checkOperandWidthsAreTheSame (statement);
      if (dstOperand.id () == ACCUMULATOR || srcOperand.id () == ACCUMULATOR) {
        statement->size (1);
      } else {
        statement->size (2);
      }
    } else {
      srcOperand.width (dstOperand.width ());
      statement->size (getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels));
    }
  } else {
    /* dstOperand.type () == Operand::Type::POINTER */
    if (dstOperand.width () == Operand::Width::UNDEFINED) {
      dstOperand.width (srcOperand.width ());
    } else {
      checkOperandWidthsAreTheSame (statement);
    }
    statement->size (getSizeOfInstructionWithPointerOperand (statement, 0, constants, labels));
  }
}

static void checkInstructionWithTwoOperandsAndSetSize (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& dstOperand = statement->operand (0);
  Operand& srcOperand = statement->operand (1);
  int instrType = statement->token (0)->subtype ();

  if (dstOperand.type () == Operand::Type::IMMEDIATE && instrType != TOKEN_SUBTYPE_TEST) {
    std::shared_ptr<Token> token = statement->token (1);
    throw ParseException ("Error in line %d, column %d: immediate operand not allowed as destination.", token->line (), token->column ());
  }
  if ((dstOperand.type () == Operand::Type::SEGMENT_REGISTER || srcOperand.type () == Operand::Type::SEGMENT_REGISTER) && instrType != TOKEN_SUBTYPE_MOV) {
    std::shared_ptr<Token> token = statement->token (0);
    throw ParseException ("Error in line %d, column %d: this instruction cannot have a segment register as operand.", token->line (), token->column ());
  }
  if (dstOperand.type () == Operand::Type::POINTER && srcOperand.type () == Operand::Type::POINTER) {
    std::shared_ptr<Token> token = statement->token (0);
    throw ParseException ("Error in line %d, column %d: an instruction cannot have two pointer operands.", token->line (), token->column ());
  }
  checkForUnknownIdentifiers (statement, constants, labels);

  switch (instrType) {
  case TOKEN_SUBTYPE_ADD:
  case TOKEN_SUBTYPE_OR:
  case TOKEN_SUBTYPE_ADC:
  case TOKEN_SUBTYPE_SBB:
  case TOKEN_SUBTYPE_AND:
  case TOKEN_SUBTYPE_SUB:
  case TOKEN_SUBTYPE_XOR:
  case TOKEN_SUBTYPE_CMP:
    checkAddAndSetSize (statement, constants, labels);
    break;

  case TOKEN_SUBTYPE_MOV:
    checkMovAndSetSize (statement, constants, labels);
    break;

  case TOKEN_SUBTYPE_TEST:
    checkTestAndSetSize (statement, constants, labels);
    break;

  case TOKEN_SUBTYPE_XCHG:
    checkXchgAndSetSize (statement, constants, labels);
    break;

  case TOKEN_SUBTYPE_ROL:
  case TOKEN_SUBTYPE_ROR:
  case TOKEN_SUBTYPE_RCL:
  case TOKEN_SUBTYPE_RCR:
  case TOKEN_SUBTYPE_SHL:
  case TOKEN_SUBTYPE_SHR:
  case TOKEN_SUBTYPE_SAL:
  case TOKEN_SUBTYPE_SAR:
    checkShiftAndSetSize (statement, constants, labels);
    break;
  }
}

static void checkIntAndSetSize (std::shared_ptr<Statement>& statement) {
  std::shared_ptr<Token>& token = statement->token (1);
  int number = token->subtype ();
  if (number <= -1 || number > 0xFF) {
    throw ParseException ("Error in line %d, column %d: the interrupt number value should be in the range [0, 0xFF].", token->line (), token->column ());
  }
  statement->size (number == 3 ? 1 : 2);
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

static void checkOutAndSetSize (std::shared_ptr<Statement>& statement) {
  Operand& dstOperand = statement->operand (0);
  if (dstOperand.type () == Operand::Type::REGISTER) {
    statement->size (1);
  } else {
    std::shared_ptr<Token>& token = statement->token (1);
    int port = token->subtype ();
    if (port <= -1 || port > 0xFF) {
      throw ParseException ("Error in line %d, column %d: the port value should be in the range [0, 0xFF].", token->line (), token->column ());
    }
    dstOperand.width (Operand::Width::BYTE);
    statement->size (2);
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
    checkInstructionWithOneOptionalOperandAndSetSize (statement);
    break;

  case Token::Type::INSTR1:
    checkInstructionWithOneOperandAndSetSize (statement, constants, labels);
    break;

  case Token::Type::INSTR2:
    checkInstructionWithTwoOperandsAndSetSize (statement, constants, labels);
    break;

  case Token::Type::LOAD:
    statement->size (getSizeOfInstructionWithPointerOperand (statement, 1, constants, labels));
    break;

  case Token::Type::IN:
    checkInAndSetSize (statement);
    break;

  case Token::Type::OUT:
    checkOutAndSetSize (statement);
    break;

  case Token::Type::INT:
    checkIntAndSetSize (statement);
    break;

  case Token::Type::JMP:
    checkJumpAndSetSize (statement, jumps, constants, labels);
    break;

  case Token::Type::CONDITIONAL_JMP:
    checkConditionalJmpAndSetSize (statement, labels);
    break;
  }
}

static int determineSizeOfJumpInstructions (
    std::vector<std::shared_ptr<Statement>>& statements,
    std::vector<std::shared_ptr<Statement>>& jumps,
    std::map<std::string, int>& labels,
    int startOffset) {

  int addedBytes = 0;
  bool resizingDone = false;
  while (!resizingDone) {
    resizingDone = true;
    for (std::shared_ptr<Statement> jump : jumps) {
      if (jump->size () == 2) {
        std::shared_ptr<Token> labelToken = jump->token (1);
        int start = jump->location () + 2;
        int end = labels.find (labelToken->text ())->second;
        int displacement = end - start;
        if (displacement <= -0x81 || displacement > 0x7F) {
          resizingDone = false;
          jump->size (3);
          ++addedBytes;
        }
      }
    }
    if (!resizingDone) {
      int offset = startOffset;
      for (std::shared_ptr<Statement> statement : statements) {
        if (statement->type () == Statement::Type::LABEL) {
          labels.find (statement->token (0)->text ())->second = offset;
        }
        statement->location (offset);
        offset += statement->size ();
      }
    }
  }
  return addedBytes;
}

static int findExpressionStart (std::shared_ptr<Statement>& statement, int operandNo) {
  int i = findTokenIndexOfOperand (statement, operandNo);
  while (!Expression::isExpressionStart (statement->token (i)->type ())) {
    ++i;
  }
  return i;
}

static int findTokenIndexOfOperand (std::shared_ptr<Statement>& statement, int operandNo) {
  int i = 1;
  if (operandNo == 1) {
    while (statement->token (i)->type () != Token::Type::COMMA) {
      ++i;
    }
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
  int displacement = calculateWordValue (statement, expressionStart, constants, labels);
  statement->operand (operandNo).displacement (displacement);
}

static void setWidthOfImmediateOperandOfAddInstruction (
    std::shared_ptr<Statement>& statement,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {

  Operand& operand = statement->operand (1);
  int expressionStart = findExpressionStart (statement, 1);
  if (canCalculateConstant (statement, expressionStart, constants)) {
    int constVal = calculateWordValue (statement, expressionStart, constants, labels);
    if (constVal <= -0x81 || constVal > 0x7F) {
      operand.width (Operand::Width::WORD);
      checkOperandWidthsAreTheSame (statement);
    } else {
      operand.width (Operand::Width::BYTE);
    }
  } else {
    operand.width (statement->operand (0).width ());
  }
}
