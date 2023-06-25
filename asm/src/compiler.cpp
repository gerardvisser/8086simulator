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
#include "exception/ParseException.h"
#include <map>

static void checkAndAddConstant (
    std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels,
    std::shared_ptr<Statement>& statement);
static std::map<std::string, int> getLabels (std::vector<std::shared_ptr<Statement>>& statements);

Compilation compiler::compile (std::istream& stream) {
  std::vector<std::shared_ptr<Statement>> statements = parser::parse (stream);
  std::map<std::string, int> labels = getLabels (statements);
  std::map<std::string, int64_t> constants;

  for (std::shared_ptr<Statement> statement : statements) {
    if (statement->type () == Statement::Type::CONSTANT) {
      checkAndAddConstant (constants, labels, statement);
    } else if (statement->type () == Statement::Type::INSTRUCTION) {
    }
  }
}

static void checkAndAddConstant (
    std::map<std::string, int64_t>& constants,
    const std::map<std::string, int>& labels,
    std::shared_ptr<Statement>& statement) {
  std::shared_ptr<Token> token = statement->token (0);
  const std::string& constant = token->text ();
  auto labelsIter = labels.find (constant);
  if (labelsIter != labels.end ()) {
    throw ParseException ("Error in line %d, column %d: a label with the name %s already exists.", token->line (), token->column (), constant.c_str ());
  }
  auto constantsIter = constants.find (constant);
  if (constantsIter != constants.end ()) {
    throw ParseException ("Error in line %d, column %d: second declaration of constant %s.", token->line (), token->column (), constant.c_str ());
  }
  for (int i = 1; i < statement->tokenCount (); ++i) {
    token = statement->token (i);
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
  constants[constant] = 0x8000000000000000;
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
