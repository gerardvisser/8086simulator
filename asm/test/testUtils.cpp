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

#include "testUtils.h"
#include "../src/Tokeniser.h"
#include <fstream>
#include <sstream>

std::vector<std::shared_ptr<Token>> testUtils::createTokens (std::istream& stream) {
  std::map<std::string, TokenData> tokenData;
  TokenData::get (tokenData);

  Tokeniser tokeniser (stream, tokenData);

  std::vector<std::shared_ptr<Token>> tokens;
  tokens.reserve (256);

  std::shared_ptr<Token> token = tokeniser.next ();
  while (token) {
    tokens.push_back (token);
    token = tokeniser.next ();
  }
  return tokens;
}

std::vector<std::shared_ptr<Token>> testUtils::createTokensFromFile (std::string filename) {
  std::ifstream stream (filename);
  return createTokens (stream);
}

std::vector<std::shared_ptr<Token>> testUtils::createTokensFromString (std::string text) {
  std::istringstream stream (text);
  return createTokens (stream);
}
