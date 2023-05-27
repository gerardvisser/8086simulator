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

#ifndef __TOKEN_INCLUDED
#define __TOKEN_INCLUDED

#include <string>

class Token {
public:
  enum class Type {
    SEGREG, REG, INSTR0, INSTR01, INSTR1, INSTR2, INT, JMP, PTR_TYPE, PTR, FAR, DB,
    IDENTIFIER, NUMBER, STRING, OPERATOR, EQUALS, COMMA, COLON, LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS, LEFT_BRACKET, RIGHT_BRACKET, UNDEFINED
  };

  enum class Error {
    NONE, HEX_NUMBER_WITHOUT_VALUE, NUMBER_TOO_LARGE, STRING_NOT_TERMINATED, STRAY_CHAR
  };

private:
  int m_line;
  int m_column;
  Type m_type;
  int m_subtype;
  std::string m_text;
  Error m_error;

public:
  Token (Type type, int line, int column);
  Token (Type type, int line, int column, int subtype);
  Token (Type type, int line, int column, std::string& text);
  Token (Type type, int line, int column, Error error);

  int column (void) const;
  Error error (void) const;
  int line (void) const;
  int subtype (void) const;
  const std::string& text (void) const;
  Type type (void) const;
};

#endif
