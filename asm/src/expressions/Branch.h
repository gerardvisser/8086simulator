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

#ifndef __BRANCH_INCLUDED
#define __BRANCH_INCLUDED

#include "Expression.h"

class Branch : public Expression {
private:
  Expression* m_leftChild;
  Expression* m_rightChild;
  const int m_precedence;

public:
  explicit Branch (int precedence);

  virtual ~Branch (void);

  const Expression* leftChild (void) const;
  void leftChild (Expression* expression);
  int precedence (void) const;
  const Expression* rightChild (void) const;
  void rightChild (Expression* expression);
};

#endif
