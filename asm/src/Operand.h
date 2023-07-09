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

#ifndef __OPERAND_INCLUDED
#define __OPERAND_INCLUDED

class Operand {
public:
  enum class Type {
    SEGMENT_REGISTER, REGISTER, POINTER, IMMEDIATE
  };

  enum class Width {
    UNDEFINED, BYTE, WORD
  };

private:
  Width m_width;
  Type m_type;
  int m_id;
  int m_displacement;

public:
  explicit Operand (Width width = Width::UNDEFINED, Type type = Type::SEGMENT_REGISTER, int id = -1);

  int displacement (void) const;
  void displacement (int val);
  int id (void) const;
  Type type (void) const;
  Width width (void) const;
  void width (Width val);
};

#endif
