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

#ifndef __EXPRESSION_INCLUDED
#define __EXPRESSION_INCLUDED

#include <cstdint>
#include <map>
#include <memory>
#include "../Token.h"

class Branch;

class Expression {
private:
  Branch* m_parent;

public:
  Expression (void);

  virtual ~Expression (void);

  static std::unique_ptr<const Expression> create (
      std::shared_ptr<Token>* tokens,
      const int tokenCount,
      const std::map<std::string, int64_t>& constants,
      const std::map<std::string, int>& labels);
  static bool isExpressionStart (Token::Type tokenType);
  static bool isExpressionToken (Token::Type tokenType);

  Branch* addBranch (char op);
  Branch* parent (void);
  virtual int64_t value (void) const = 0;

  friend class Branch;
};

#endif
