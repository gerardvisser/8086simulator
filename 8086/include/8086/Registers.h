/*
   Author:  Gerard Visser
   e-mail:  visser.gerard(at)gmail.com

   Copyright (C) 2024 Gerard Visser.

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

#ifndef __REGISTERS_INCLUDED
#define __REGISTERS_INCLUDED

#include <cstdint>

class Registers {
public:
  uint16_t gen[8];
  uint16_t seg[4];
  uint16_t flags;
  uint16_t ip;

  Registers (void);
  uint16_t getGen (bool wide, int index);
  void setGen (bool wide, int index, uint16_t value);
};

#endif
