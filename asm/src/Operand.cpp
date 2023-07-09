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

#include "Operand.h"

Operand::Operand (Width width, Type type, int id) : m_width (width), m_type (type), m_id (id), m_displacement (0) {
}

int Operand::displacement (void) const {
  return m_displacement;
}

void Operand::displacement (int val) {
  m_displacement = val;
}

int Operand::id (void) const {
  return m_id;
}

Operand::Type Operand::type (void) const {
  return m_type;
}

Operand::Width Operand::width (void) const {
  return m_width;
}

void Operand::width (Width val) {
  m_width = val;
}
