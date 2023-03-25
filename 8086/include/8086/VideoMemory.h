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

#ifndef __VIDEO_MEMORY_INCLUDED
#define __VIDEO_MEMORY_INCLUDED

#include <cstdint>

class VideoMemory {
private:
  uint32_t* m_buffer;
  uint32_t m_latch;
  uint8_t m_setReset;
  uint8_t m_enableSetReset;
  uint8_t m_colourCompare;
  uint8_t m_logicalOperation;
  uint8_t m_rotateCount;
  uint8_t m_readPlaneSelect;
  uint8_t m_readMode;
  uint8_t m_writeMode;
  uint8_t m_colourDontCare;
  uint8_t m_bitMask;
  uint8_t m_writePlaneEnable;
  uint8_t m_memoryMapSelect;

public:
  VideoMemory (void);

  VideoMemory (const VideoMemory&) = delete;
  VideoMemory (VideoMemory&&) = delete;

  ~VideoMemory (void);

  VideoMemory& operator= (const VideoMemory&) = delete;
  VideoMemory& operator= (VideoMemory&&) = delete;

  int readByte (int index);
  void readBytes (uint8_t* dst, int index, uint16_t count);
  int readWord (int index);
  void writeByte (int index, uint8_t val);
  void writeBytes (int index, const uint8_t* src, uint16_t count);
  void writeWord (int index, int val);

  uint8_t bitMask (void) const;
  void bitMask (uint8_t val);
  uint8_t colourCompare (void) const;
  void colourCompare (uint8_t val);
  uint8_t colourDontCare (void) const;
  void colourDontCare (uint8_t val);
  uint8_t enableSetReset (void) const;
  void enableSetReset (uint8_t val);
  uint8_t logicalOperation (void) const;
  void logicalOperation (uint8_t val);
  uint8_t memoryMapSelect (void) const;
  void memoryMapSelect (uint8_t val);
  uint8_t readMode (void) const;
  void readMode (uint8_t val);
  uint8_t readPlaneSelect (void) const;
  void readPlaneSelect (uint8_t val);
  uint8_t rotateCount (void) const;
  void rotateCount (uint8_t val);
  uint8_t setReset (void) const;
  void setReset (uint8_t val);
  uint8_t writeMode (void) const;
  void writeMode (uint8_t val);
  uint8_t writePlaneEnable (void) const;
  void writePlaneEnable (uint8_t val);

private:
  int translateIndex (int index) const;


#ifdef TEST_MODE
public:
  uint32_t* buffer (void);
#endif
};

#endif
