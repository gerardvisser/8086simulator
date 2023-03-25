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
#include <8086/VideoMemory.h>

#define MEM_SIZE 0x10000

static const uint32_t colcmp[] = {
  0xFFFFFFFF, 0xFFFFFF00, 0xFFFF00FF, 0xFFFF0000, 0xFF00FFFF, 0xFF00FF00, 0xFF0000FF, 0xFF000000,
  0x00FFFFFF, 0x00FFFF00, 0x00FF00FF, 0x00FF0000, 0x0000FFFF, 0x0000FF00, 0x000000FF, 0x00000000
};

VideoMemory::VideoMemory (void) {
  m_buffer = (uint32_t*) malloc (sizeof (uint32_t) * MEM_SIZE);
  /* TODO: Clear memory? */
  m_latch = 0;
  m_setReset = 0;
  m_enableSetReset = 0;
  m_colourCompare = 0;
  m_logicalOperation = 0;
  m_rotateCount = 0;
  m_readPlaneSelect = 0;
  m_readMode = 0;
  m_writeMode = 0;
  m_colourDontCare = 0xF;
  m_bitMask = 0xFF;
  m_writePlaneEnable = 0xF;
  m_memoryMapSelect = 1;
}

VideoMemory::~VideoMemory (void) {
  free (m_buffer);
}

int VideoMemory::readByte (int index) {
  index = translateIndex (index);
  if (index < 0) {
    return 0xFF;
  }

  uint8_t result;
  m_latch = m_buffer[index];
  if (m_readMode != 0) {
    uint32_t tmp = m_latch ^ colcmp[m_colourCompare];
    tmp |= colcmp[m_colourDontCare];
    result = tmp >> 24;
    result &= tmp >> 16;
    result &= tmp >> 8;
    result &= tmp;
  } else {
    result = m_latch >> 8 * m_readPlaneSelect;
  }
  return result;
}

void VideoMemory::readBytes (uint8_t* dst, int index, uint16_t count) {
  for (int i = 0; i < count; ++i) {
    dst[i] = readByte (index + i);
  }
}

int VideoMemory::readWord (int index) {
  int result = readByte (index + 1);
  result <<= 8;
  result |= readByte (index);
  return result;
}

void VideoMemory::writeByte (int index, uint8_t val) {
  //TODO
}

void VideoMemory::writeBytes (int index, const uint8_t* src, uint16_t count) {
  for (int i = 0; i < count; ++i) {
    writeByte (index + i, src[i]);
  }
}

void VideoMemory::writeWord (int index, int val) {
  writeByte (index, val);
  writeByte (index + 1, val >> 8);
}

/* 3CE, 8 */
uint8_t VideoMemory::bitMask (void) const {
  return m_bitMask;
}

void VideoMemory::bitMask (uint8_t val) {
  m_bitMask = val;
}

/* 3CE, 2 */
uint8_t VideoMemory::colourCompare (void) const {
  return m_colourCompare;
}

void VideoMemory::colourCompare (uint8_t val) {
  m_colourCompare = val & 0xF;
}

/* 3CE, 7 */
uint8_t VideoMemory::colourDontCare (void) const {
  return m_colourDontCare;
}

void VideoMemory::colourDontCare (uint8_t val) {
  m_colourDontCare = val & 0xF;
}

/* 3CE, 1 */
uint8_t VideoMemory::enableSetReset (void) const {
  return m_enableSetReset;
}

void VideoMemory::enableSetReset (uint8_t val) {
  m_enableSetReset = val & 0xF;
}

/* 3CE, 3, bits: 4, 3 */
uint8_t VideoMemory::logicalOperation (void) const {
  return m_logicalOperation;
}

void VideoMemory::logicalOperation (uint8_t val) {
  m_logicalOperation = val & 0x3;
}

/* 3CE, 6, bits: 3, 2 */
uint8_t VideoMemory::memoryMapSelect (void) const {
  return m_memoryMapSelect;
}

void VideoMemory::memoryMapSelect (uint8_t val) {
  m_memoryMapSelect = val & 0x3;
}

/* 3CE, 5, bit: 3 */
uint8_t VideoMemory::readMode (void) const {
  return m_readMode;
}

void VideoMemory::readMode (uint8_t val) {
  m_readMode = val & 0x1;
}

/* 3CE, 4 */
uint8_t VideoMemory::readPlaneSelect (void) const {
  return m_readPlaneSelect;
}

void VideoMemory::readPlaneSelect (uint8_t val) {
  m_readPlaneSelect = val & 0x3;
}

/* 3CE, 3, bits: 2, 1, 0 */
uint8_t VideoMemory::rotateCount (void) const {
  return m_rotateCount;
}

void VideoMemory::rotateCount (uint8_t val) {
  m_rotateCount = val & 0x7;
}

/* 3CE, 0 */
uint8_t VideoMemory::setReset (void) const {
  return m_setReset;
}

void VideoMemory::setReset (uint8_t val) {
  m_setReset = val & 0xF;
}

/* 3CE, 5, bits: 1, 0 */
uint8_t VideoMemory::writeMode (void) const {
  return m_writeMode;
}

void VideoMemory::writeMode (uint8_t val) {
  m_writeMode = val & 0x3;
}

/* 3C4, 2 */
uint8_t VideoMemory::writePlaneEnable (void) const {
  return m_writePlaneEnable;
}

void VideoMemory::writePlaneEnable (uint8_t val) {
  m_writePlaneEnable = val & 0xF;
}


int VideoMemory::translateIndex (int index) const {
  switch (m_memoryMapSelect) {
  case 0: // 00000 - 1FFFF
    return index;

  case 1: // 00000 - 0FFFF
    return index > 0xFFFF ? -1 : index;

  case 2: // 10000 - 17FFF
    index -= 0x10000;
    return index > 0x7FFF ? -1 : index;

  default: // 18000 - 1FFFF
    index -= 0x18000;
    return index > 0x7FFF ? -1 : index;
  }
}


#ifdef TEST_MODE

uint32_t* VideoMemory::buffer (void) {
  return m_buffer;
}

#endif
