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

#ifndef __ADDRESS_INCLUDED
#define __ADDRESS_INCLUDED

#include <cstdint>

class Address {
public:
  uint16_t segment;
  uint16_t offset;

  explicit Address (uint16_t segment = 0, uint16_t offset = 0);

  operator int (void) const;
  Address operator+ (int x) const;
  Address operator- (int x) const;
  Address& operator+= (int x);
  Address& operator-= (int x);
  Address& operator++ (void);
  Address operator++ (int);
  Address& operator-- (void);
  Address operator-- (int);
};

Address operator+ (int x, Address address);
Address operator- (int x, Address address);

#endif
