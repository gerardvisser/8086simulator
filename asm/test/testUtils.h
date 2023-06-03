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

#ifndef __TEST_UTILS_INCLUDED
#define __TEST_UTILS_INCLUDED

#include <istream>
#include <memory>
#include <vector>
#include "../src/Token.h"

namespace testUtils {
  std::vector<std::shared_ptr<Token>> createTokens (std::istream& stream);
  std::vector<std::shared_ptr<Token>> createTokensFromFile (std::string filename);
  std::vector<std::shared_ptr<Token>> createTokensFromString (std::string text);
}

#endif
