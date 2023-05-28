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

#include <algorithm>
#include "Statement.h"

Statement::Statement (Type type, std::vector<std::shared_ptr<Token>>& tokens) : m_type (type), m_tokenCount (tokens.size ()) {
  m_tokens = new std::shared_ptr<Token>[m_tokenCount];
  std::copy (tokens.begin (), tokens.end (), m_tokens);
}

Statement::~Statement (void) {
  delete[] m_tokens;
}

std::shared_ptr<Token>& Statement::token (int index) const {
  return m_tokens[index];
}

int Statement::tokenCount (void) const {
  return m_tokenCount;
}

Statement::Type Statement::type (void) const {
  return m_type;
}


//Builder

Statement::Builder::Builder (void) {
  m_tokens.reserve (256);
  m_type = Type::INSTRUCTION;
}

Statement::Builder& Statement::Builder::addToken (std::shared_ptr<Token> token) {
  m_tokens.push_back (token);
  return *this;
}

std::shared_ptr<Statement> Statement::Builder::build (void) {
  Statement* statement = new Statement (m_type, m_tokens);
  m_tokens.clear ();
  return std::shared_ptr<Statement> (statement);
}

Statement::Builder& Statement::Builder::type (Type type) {
  m_type = type;
  return *this;
}
