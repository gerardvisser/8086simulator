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

#ifndef __MEMORY_INCLUDED
#define __MEMORY_INCLUDED

#include <8086/Address.h>

class Memory {
private:
  uint8_t* m_buffer;

public:
  Memory (void);

  Memory (const Memory&) = delete;
  Memory (Memory&&) = delete;

  ~Memory (void);

  Memory& operator= (const Memory&) = delete;
  Memory& operator= (Memory&&) = delete;

  int read (Address address, bool wide = false) const;
  int readByte (Address address) const;
  void readBytes (uint8_t* dst, Address address, uint16_t count) const;
  int readSignedByteAsWord (Address address) const;
  int readWord (Address address) const;
  void write (Address address, int val, bool wide = false);
  void writeByte (Address address, int val);
  void writeBytes (Address address, const uint8_t* src, uint16_t count);
  void writeWord (Address address, int val);
};

#endif
