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

#include "Branch.h"

Branch::Branch (int precedence) : m_precedence (precedence), m_leftChild (nullptr), m_rightChild (nullptr) {
}

Branch::~Branch (void) {
  if (m_leftChild != nullptr) {
    delete m_leftChild;
  }
  if (m_rightChild != nullptr) {
    delete m_rightChild;
  }
}

const Expression* Branch::leftChild (void) const {
  return m_leftChild;
}

void Branch::leftChild (Expression* expression) {
  if (expression != nullptr) {
    expression->m_parent = this;
  }
  m_leftChild = expression;
}

int Branch::precedence (void) const {
  return m_precedence;
}

const Expression* Branch::rightChild (void) const {
  return m_rightChild;
}

void Branch::rightChild (Expression* expression) {
  if (expression != nullptr) {
    expression->m_parent = this;
  }
  m_rightChild = expression;
}
