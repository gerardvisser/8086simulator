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

#include <8086/Registers.h>

Registers::Registers (void) {
  for (int i = 0; i < 8; ++i) {
    gen[i] = 0;
  }
  for (int i = 0; i < 4; ++i) {
    seg[i] = 0;
  }
  flags = 0xF002;
  ip = 0;
}

uint16_t Registers::getGen (bool wide, int index) {
  if (wide) {
    index &= 7;
    return gen[index];
  }

  bool high = (index & 4) != 0;
  index &= 3;
  uint16_t value = gen[index];
  if (high) {
    value >>= 8;
  } else {
    value &= 0xFF;
  }
  return value;
}

void Registers::setGen (bool wide, int index, uint16_t value) {
  if (wide) {
    index &= 7;
    gen[index] = value;
  } else {
    bool high = (index & 4) != 0;
    index &= 3;
    if (high) {
      value <<= 8;
      gen[index] &= 0x00FF;
    } else {
      value &= 0xFF;
      gen[index] &= 0xFF00;
    }
    gen[index] |= value;
  }
}
