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

#include <cstdlib>
#include <8086/Memory.h>

#define HIGH_AREA_START  (VIDEO_AREA_START + VIDEO_AREA_SIZE)
#define MEM_SIZE         0x100000
#define VIDEO_AREA_SIZE  0x20000
#define VIDEO_AREA_START 0xA0000

Memory::Memory (void) {
  m_buffer = (uint8_t*) malloc (MEM_SIZE - VIDEO_AREA_SIZE);
}

Memory::~Memory (void) {
  free (m_buffer);
}

int Memory::read (Address address, bool wide) const {
}

int Memory::readByte (Address address) const {
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
    return m_buffer[index];

  case 5:
    //result = m_videoMem->readByte (index - VIDEO_AREA_START);
    return 0;

  default:
    index -= VIDEO_AREA_SIZE;
    return m_buffer[index];
  }
}

void Memory::readBytes (uint8_t* dst, Address address, uint16_t count) const {
}

int Memory::readSignedByteAsWord (Address address) const {
}

int Memory::readWord (Address address) const {
}

void Memory::write (Address address, int val, bool wide) {
}

void Memory::writeByte (Address address, int val) {
}

void Memory::writeBytes (Address address, const uint8_t* src, uint16_t count) {
}

void Memory::writeWord (Address address, int val) {
}
