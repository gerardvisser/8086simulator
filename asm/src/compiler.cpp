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
#include <map>

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
static std::map<std::string, int> getLabels (std::vector<std::shared_ptr<Statement>>& statements);
static void setDataStatementSize (std::shared_ptr<Statement>& statement);

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
  constants[constant] = 0x8000000000000000;
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

static void checkJumpAndSetSize (
    std::shared_ptr<Statement>& statement,
    std::vector<std::shared_ptr<Statement>>& jumps,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  if (statement->operandCount () == 1) {
    Operand& operand = statement->operand (0);
    if (operand.type () == Operand::Type::POINTER) {
      checkForUnknownIdentifiers (statement, constants, labels);
      if (operand.id () > 7) {
        /* TODO:
           i) check displacement doesn't contain labels
           ii) determine displacement

           Dit betekent ook dat de constanten die in de displacement-expressie mogen voorkomen zelf ook
           niet van labels af mogen hangen. Je moet eigenlijk de constanten bij het aanmaken al checken
           op labels en als ze die niet hebben meteen uitrekenen.  Als een constante dan nog de waarde 0x8000000000000000
           heeft in de map, dan is zij afhankelijk van een of meer labels. */
      } else if (operand.id () == 6) {
        statement->size (4);
      } else {
        statement->size (2);
      }
    } else {
      statement->size (2);
    }
  } else if (statement->token (1)->type () == Token::Type::IDENTIFIER) {
    std::shared_ptr<Token>& token = statement->token (1);
    if (labels.find (token->text ()) == labels.end ()) {
      throw ParseException ("Error in line %d, column %d: unknown label %s.", token->line (), token->column (), token->text ().c_str ());
    }
    if (statement->token (0)->subtype () == TOKEN_SUBTYPE_JMP) {
      jumps.push_back (statement);
      statement->size (2);
    } else {
      statement->size (3);
    }
  } else {
    statement->size (5);
  }
}

static void checkInstructionAndSetSize (
    std::shared_ptr<Statement>& statement,
    std::vector<std::shared_ptr<Statement>>& jumps,
    const std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels) {
  std::shared_ptr<Token> token = statement->token (0);//TODO: ref?
  switch (token->type ()) {
  case Token::Type::SEGREG:
  case Token::Type::INSTR0:
    statement->size (1);
    break;

  case Token::Type::INSTR01:
    break;

  case Token::Type::INSTR1:
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
