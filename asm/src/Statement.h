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

#ifndef __STATEMENT_INCLUDED
#define __STATEMENT_INCLUDED

#include <memory>
#include <vector>
#include "Operand.h"
#include "Token.h"

class Statement {
public:
  enum class Type {
    LABEL, CONSTANT, INSTRUCTION, DATA
  };

private:
  std::shared_ptr<Token>* m_tokens;
  Operand m_operands[2];
  int m_operandCount;
  int m_tokenCount;
  Type m_type;
  int m_size;
  int m_location;

  Statement (Type type, std::vector<std::shared_ptr<Token>>& tokens, std::vector<Operand>& operands);

public:
  Statement (const Statement&) = delete;
  Statement (Statement&&) = delete;

  ~Statement (void);

  Statement& operator= (const Statement&) = delete;
  Statement& operator= (Statement&&) = delete;

  int location (void) const;
  void location (int val);
  Operand& operand (int index);
  int operandCount (void) const;
  int size (void) const;
  void size (int val);
  std::shared_ptr<Token>& token (int index) const;
  int tokenCount (void) const;
  std::shared_ptr<Token>* tokens (void) const;
  Type type (void) const;

  class Builder {
  private:
    std::vector<std::shared_ptr<Token>> m_tokens;
    std::vector<Operand> m_operands;
    Type m_type;

  public:
    Builder (void);

    Builder (const Builder&) = delete;
    Builder (Builder&&) = delete;

    Builder& operator= (const Builder&) = delete;
    Builder& operator= (Builder&&) = delete;

    Builder& addOperand (Operand operand);
    Builder& addToken (std::shared_ptr<Token> token);
    std::shared_ptr<Statement> build (void);
    Builder& type (Type type);
  };
};

#endif
