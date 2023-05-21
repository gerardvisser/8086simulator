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

#ifndef __TOKEN_DATA_INCLUDED
#define __TOKEN_DATA_INCLUDED

#include <map>
#include "Token.h"

class TokenData {
private:
  Token::Type m_type;
  int m_subtype;

public:
  explicit TokenData (Token::Type type = Token::Type::SEGREG, int subtype = -1);

  static void get (std::map<std::string, TokenData>& tokenData);

  int subtype (void) const;
  Token::Type type (void) const;
};

#endif
