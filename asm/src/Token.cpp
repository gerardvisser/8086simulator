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

#include "Token.h"

Token::Token (Type type, int line, int column) : m_type (type), m_line (line), m_column (column), m_subtype (0), m_text (), m_error (Error::NONE) {
}

Token::Token (Type type, int line, int column, int subtype) : m_type (type), m_line (line), m_column (column), m_subtype (subtype), m_text (), m_error (Error::NONE) {
}

Token::Token (Type type, int line, int column, std::string& text) : m_type (type), m_line (line), m_column (column), m_subtype (0), m_text (text), m_error (Error::NONE) {
}

Token::Token (Type type, int line, int column, Error error) : m_type (type), m_line (line), m_column (column), m_subtype (0), m_text (), m_error (error) {
}

int Token::column (void) const {
  return m_column;
}

Token::Error Token::error (void) const {
  return m_error;
}

int Token::line (void) const {
  return m_line;
}

int Token::subtype (void) const {
  return m_subtype;
}

const std::string& Token::text (void) const {
  return m_text;
}

Token::Type Token::type (void) const {
  return m_type;
}
