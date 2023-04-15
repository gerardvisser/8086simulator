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
#include <cstring>
#include <8086/VideoMemory.h>

#define MEM_SIZE 0x10000

/* Memory modes: */
#define ODD_EVEN              0
#define PLANAR                1
#define CHAIN_4               3
#define UNDEFINED_MEMORY_MODE 2

/* Shift modes: */
#define SINGLE_SHIFT         0
#define INTERLEAVED_SHIFT    1
#define _256_SHIFT           2
#define UNDEFINED_SHIFT_MODE 3

static const uint32_t colcmp[] = {
  0xFFFFFFFF, 0xFFFFFF00, 0xFFFF00FF, 0xFFFF0000, 0xFF00FFFF, 0xFF00FF00, 0xFF0000FF, 0xFF000000,
  0x00FFFFFF, 0x00FFFF00, 0x00FF00FF, 0x00FF0000, 0x0000FFFF, 0x0000FF00, 0x000000FF, 0x00000000
};

VideoMemory::VideoMemory (void) {
  m_buffer = (uint32_t*) malloc (sizeof (uint32_t) * MEM_SIZE);
  memset (m_buffer, 0, sizeof (uint32_t) * MEM_SIZE);
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
  m_memoryMode = PLANAR;
  m_shiftMode = SINGLE_SHIFT;
  m_cgaAddressing = false;
  m_graphicsMode = true;
  m_updated = true;
}

VideoMemory::~VideoMemory (void) {
  free (m_buffer);
}

/*  Characters are always 8 pixels wide.
    @param dst the destination array, which should be at least 512kiB big.
    @param widthInCharacters the width in characters of a line; in 256 colour modes the value
        should be doubled, which is how it is already stored in register 3D4, 1 (except that
        the value in that register is one too few).
    @param heightInScanLines the height in scanlines.
    @return true if the pixels were created.  */
bool VideoMemory::getPixels (uint8_t* dst, int widthInCharacters, int heightInScanLines) {
  if (!m_updated) {
    return false;
  }
  m_updated = false;

  if (!m_graphicsMode) {
    /* TODO: alphanumeric mode. */
  }
  if (m_cgaAddressing) {
    getPixelsCgaAddressing (dst, widthInCharacters, heightInScanLines);
    return true;
  }

  int count = widthInCharacters * heightInScanLines;
  switch (m_shiftMode) {
  case SINGLE_SHIFT:
    getPixelsSingleShift (dst, 0, 0, count);
    break;

  case INTERLEAVED_SHIFT:
    getPixelsInterleavedShift (dst, 0, 0, count);
    break;

  default:
    getPixels256Shift (dst, 0, 0, count);
  }
  return true;
}

int VideoMemory::readByte (int index) {
  index = translateIndex (index);
  if (index < 0) {
    return 0xFF;
  }

  int readMode;
  int readPlaneSelect;
  switch (m_memoryMode) {
  case ODD_EVEN:
    readMode = 0;
    readPlaneSelect = index & 1;
    readPlaneSelect |= m_readPlaneSelect & 2;
    index >>= 1;
    break;

  case PLANAR:
    readMode = m_readMode;
    readPlaneSelect = m_readPlaneSelect;
    break;

  default:
    readMode = 0;
    readPlaneSelect = index & 3;
    index >>= 2;
  }

  uint8_t result;
  m_latch = m_buffer[index];
  if (readMode != 0) {
    uint32_t tmp = m_latch ^ colcmp[m_colourCompare];
    tmp |= colcmp[m_colourDontCare];
    result = tmp >> 24;
    result &= tmp >> 16;
    result &= tmp >> 8;
    result &= tmp;
  } else {
    result = m_latch >> 8 * readPlaneSelect;
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
  index = translateIndex (index);
  if (index < 0) {
    return;
  }

  int writeMode;
  int writePlaneEnable;
  switch (m_memoryMode) {
  case ODD_EVEN:
    writeMode = 4;
    writePlaneEnable = 5 << (index & 1) & m_writePlaneEnable;
    index >>= 1;
    break;

  case PLANAR:
    writeMode = m_writeMode;
    writePlaneEnable = m_writePlaneEnable;
    break;

  default:
    writeMode = 4;
    writePlaneEnable = 1 << (index & 3) & m_writePlaneEnable;
    index >>= 2;
  }

  uint32_t planes;
  switch (writeMode) {
  case 0:
    val = ror (val);
    planes = replicate (val);
    planes = setResetPlanes (planes, m_enableSetReset);
    planes = applyLogOp (planes);
    planes = applyMask (planes, m_bitMask);
    break;

  case 1:
    planes = m_latch;
    break;

  case 2:
    planes = colcmp[15 - (val & 0xF)];
    planes = applyLogOp (planes);
    planes = applyMask (planes, m_bitMask);
    break;

  case 3:
    val = ror (val);
    val &= m_bitMask;
    planes = setResetPlanes (0, 0xF);
    planes = applyLogOp (planes); /* According to docs on internet about VGA, this operation should not be performed.  */
    planes = applyMask (planes, val);
    break;

  case 4:
    planes = replicate (val);
  }

  m_buffer[index] &= colcmp[writePlaneEnable];
  m_buffer[index] |= planes & colcmp[15 - writePlaneEnable];
  m_updated = true;
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

/* 3D4, 17, bit: 0, inverted */
bool VideoMemory::cgaAddressing (void) const {
  return m_cgaAddressing;
}

void VideoMemory::cgaAddressing (bool val) {
  m_cgaAddressing = val;
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

/* 3CE, 6, bit: 0
   Note that this bit will determine the value of readonly bit: 3C0, 10, bit: 0 */
bool VideoMemory::graphicsMode (void) const {
  return m_graphicsMode;
}

void VideoMemory::graphicsMode (bool val) {
  m_graphicsMode = val;
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

/* 3C4, 4, bits: 3, 2
   Note that bit 2 will determine the value of readonly bits:
   3CE, 5, bit 4 and 3CE, 6, bit 1, in both cases the opposite
   value of 3C4, 4, bit 2.  */
uint8_t VideoMemory::memoryMode (void) const {
  return m_memoryMode;
}

void VideoMemory::memoryMode (uint8_t val) {
  val &= 0x3;
  if (val == UNDEFINED_MEMORY_MODE) {
    val = CHAIN_4;
  }
  m_memoryMode = val;
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

/* 3CE, 5, bits: 6, 5 */
uint8_t VideoMemory::shiftMode (void) const {
  return m_shiftMode;
}

void VideoMemory::shiftMode (uint8_t val) {
  val &= 0x3;
  if (val == UNDEFINED_SHIFT_MODE) {
    val = _256_SHIFT;
  }
  m_shiftMode = val;
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


uint32_t VideoMemory::applyLogOp (uint32_t planes) const {
  switch (m_logicalOperation) {
  case 1:
    planes &= m_latch;
    break;
  case 2:
    planes |= m_latch;
    break;
  case 3:
    planes ^= m_latch;
  }
  return planes;
}

uint32_t VideoMemory::applyMask (uint32_t planes, uint8_t mask) const {
  uint32_t wideMask = replicate (mask);
  planes &= wideMask;
  planes |= m_latch & (wideMask ^ 0xFFFFFFFF);
  return planes;
}

void VideoMemory::getPixels256Shift (uint8_t* dst, int dstOff, int srcOff, int count) const {
  for (int i = 0; i < count; ++i) {
    int k = 0;
    for (int j = 0; j < 4; ++j) {
      dst[dstOff] = m_buffer[srcOff] >> k;
      dst[dstOff + 1] = dst[dstOff];
      dst[dstOff] >>= 4;
      dst[dstOff + 1] &= 0x0F;
      dstOff += 2;
      k += 8;
    }
    ++srcOff;
  }
}

void VideoMemory::getPixelsCgaAddressing (uint8_t* dst, int widthInCharacters, int heightInScanLines) const {
  void (VideoMemory::*getPixels) (uint8_t*, int, int, int) const;
  switch (m_shiftMode) {
  case SINGLE_SHIFT:
    getPixels = &VideoMemory::getPixelsSingleShift;
    break;
  case INTERLEAVED_SHIFT:
    getPixels = &VideoMemory::getPixelsInterleavedShift;
    break;
  default:
    getPixels = &VideoMemory::getPixels256Shift;
  }

  int offset8kiB = 0x2000;
  if (m_memoryMode == ODD_EVEN) {
    offset8kiB >>= 1;
  } else if (m_memoryMode == CHAIN_4) {
    offset8kiB >>= 2;
  }

  int dstOff = 0;
  int srcOff = 0;
  for (int scanLine = 0; scanLine < heightInScanLines; ++scanLine) {
    (this->*getPixels) (dst, dstOff, srcOff, widthInCharacters);
    dstOff += widthInCharacters << 3;
    srcOff += widthInCharacters;
    if ((scanLine & 1) != 0) {
      srcOff -= offset8kiB;
    } else {
      srcOff += offset8kiB - widthInCharacters;
    }
  }
}

void VideoMemory::getPixelsInterleavedShift (uint8_t* dst, int dstOff, int srcOff, int count) const {
  for (int i = 0; i < count; ++i) {
    uint32_t mask3 = 0x80000000;
    uint32_t mask2 = 0x800000;
    uint32_t mask1 = 0x8000;
    uint32_t mask0 = 0x80;
    for (int j = 0; j < 4; ++j) {
      dst[dstOff] = (m_buffer[srcOff] & mask2) != 0;
      dst[dstOff] <<= 1;
      mask2 >>= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask2) != 0;
      dst[dstOff] <<= 1;
      mask2 >>= 1;

      dst[dstOff] |= (m_buffer[srcOff] & mask0) != 0;
      dst[dstOff] <<= 1;
      mask0 >>= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask0) != 0;
      mask0 >>= 1;
      ++dstOff;
    }
    for (int j = 0; j < 4; ++j) {
      dst[dstOff] = (m_buffer[srcOff] & mask3) != 0;
      dst[dstOff] <<= 1;
      mask3 >>= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask3) != 0;
      dst[dstOff] <<= 1;
      mask3 >>= 1;

      dst[dstOff] |= (m_buffer[srcOff] & mask1) != 0;
      dst[dstOff] <<= 1;
      mask1 >>= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask1) != 0;
      mask1 >>= 1;
      ++dstOff;
    }
    ++srcOff;
  }
}

void VideoMemory::getPixelsSingleShift (uint8_t* dst, int dstOff, int srcOff, int count) const {
  for (int i = 0; i < count; ++i) {
    uint32_t mask3 = 0x80000000;
    uint32_t mask2 = 0x800000;
    uint32_t mask1 = 0x8000;
    uint32_t mask0 = 0x80;
    for (int j = 0; j < 8; ++j) {
      dst[dstOff] = (m_buffer[srcOff] & mask3) != 0;
      dst[dstOff] <<= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask2) != 0;
      dst[dstOff] <<= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask1) != 0;
      dst[dstOff] <<= 1;
      dst[dstOff] |= (m_buffer[srcOff] & mask0) != 0;
      mask3 >>= 1;
      mask2 >>= 1;
      mask1 >>= 1;
      mask0 >>= 1;
      ++dstOff;
    }
    ++srcOff;
  }
}

uint32_t VideoMemory::replicate (uint8_t value) {
  uint32_t result = value;
  result <<= 8;
  result |= value;
  result <<= 8;
  result |= value;
  result <<= 8;
  result |= value;
  return result;
}

uint8_t VideoMemory::ror (uint8_t value) const {
  int result = value;
  result <<= 8;
  result |= value;
  result >>= m_rotateCount;
  return result;
}

uint32_t VideoMemory::setResetPlanes (uint32_t planes, int enable) const {
  planes &= colcmp[enable];
  planes |= colcmp[15 - (enable & m_setReset)];
  return planes;
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
