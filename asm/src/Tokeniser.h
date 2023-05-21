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

#ifndef __TOKENISER_INCLUDED
#define __TOKENISER_INCLUDED

#include <istream>
#include <memory>
#include "Token.h"
#include "TokenData.h"

class Tokeniser {
private:
  std::istream& m_stream;
  std::map<std::string, TokenData>& m_tokenData;
  std::string m_buffer;
  int m_line;
  int m_column;
  bool m_minusPartOfNumber;

public:
  Tokeniser (std::istream& stream, std::map<std::string, TokenData>& tokenData);

  Tokeniser (const Tokeniser&) = delete;
  Tokeniser (Tokeniser&&) = delete;

  Tokeniser& operator= (const Tokeniser&) = delete;
  Tokeniser& operator= (Tokeniser&&) = delete;

  std::shared_ptr<Token> next (void);

private:
  std::shared_ptr<Token> createHexNumberToken (bool negate);
  std::shared_ptr<Token> createIDToken (int c);
  std::shared_ptr<Token> createNumberToken (int c, bool negate);
  std::shared_ptr<Token> createOtherToken (int c);
  std::shared_ptr<Token> createStringToken (void);
  static bool isDigit (int c);
  static bool isHexDigit (int c);
  static bool isIDChar (int c);
  static bool isIDFirstChar (int c);
  static bool isLetter (int c);
  static bool isWhitespace (int c);
  int skipWhitespaceAndComments (int c);
};

#endif
