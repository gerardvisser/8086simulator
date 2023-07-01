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

#include "DivBranch.h"
#include "precedence.h"
#include "../exception/RuntimeException.h"

DivBranch::DivBranch (void) : Branch (DIV_PRECEDENCE) {
}

DivBranch::~DivBranch (void) {
}

int64_t DivBranch::value (void) const {
  const Expression* left = leftChild ();
  const Expression* right = rightChild ();
  if (left == nullptr || right == nullptr) {
    throw RuntimeException ("%s:%d: leftChild or rightChild is null.", __FILE__, __LINE__);
  }
  return left->value () / right->value ();
}
