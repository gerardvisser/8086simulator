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
#include <8086/Memory.h>
#include <8086/exception/RuntimeException.h>

#define HIGH_AREA_START  (VIDEO_AREA_START + VIDEO_AREA_SIZE)
#define MEM_SIZE         0x100000
#define VIDEO_AREA_SIZE  0x20000
#define VIDEO_AREA_START 0xA0000

Memory::Memory (VideoMemory& videoMemory) : m_videoMemory (videoMemory) {
  m_buffer = (uint8_t*) malloc (MEM_SIZE - VIDEO_AREA_SIZE);
}

Memory::~Memory (void) {
  free (m_buffer);
}

int Memory::read (Address address, bool wide) const {
  if (wide) {
    return readWord (address);
  }
  return readByte (address);
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
    return m_videoMemory.readByte (index - VIDEO_AREA_START);

  default:
    return m_buffer[index - VIDEO_AREA_SIZE];
  }
}

void Memory::readBytes (uint8_t* dst, Address address, uint16_t count) const {
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
    memcpy (dst, m_buffer + index, count);
    break;

  case 4:
    if (index + count <= VIDEO_AREA_START) {
      memcpy (dst, m_buffer + index, count);
    } else {
      memcpy (dst, m_buffer + index, VIDEO_AREA_START - index);
      m_videoMemory.readBytes (dst + VIDEO_AREA_START - index, 0, count - VIDEO_AREA_START + index);
    }
    break;

  case 5:
    if (index + count <= HIGH_AREA_START) {
      m_videoMemory.readBytes (dst, index - VIDEO_AREA_START, count);
    } else {
      m_videoMemory.readBytes (dst, index - VIDEO_AREA_START, HIGH_AREA_START - index);
      memcpy (dst + HIGH_AREA_START - index, m_buffer + VIDEO_AREA_START, count - (HIGH_AREA_START - index));
    }
    break;

  default:
    if (index + count > MEM_SIZE) {
      throw RuntimeException ("%s:%d: index out of bounds.", __FILE__, __LINE__);
    }
    memcpy (dst, m_buffer + index - VIDEO_AREA_SIZE, count);
  }
}

int Memory::readSignedByteAsWord (Address address) const {
  int result = (int8_t) readByte (address);
  result &= 0xFFFF;
  return result;
}

int Memory::readWord (Address address) const {
  int result;
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
    result = m_buffer[index + 1];
    result <<= 8;
    result |= m_buffer[index];
    break;

  case 4:
    if (index < VIDEO_AREA_START - 1) {
      result = m_buffer[index + 1];
    } else {
      result = m_videoMemory.readByte (0);
    }
    result <<= 8;
    result |= m_buffer[index];
    break;

  case 5:
    if (index < HIGH_AREA_START - 1) {
      result = m_videoMemory.readWord (index - VIDEO_AREA_START);
    } else {
      result = m_buffer[VIDEO_AREA_START];
      result <<= 8;
      result |= m_videoMemory.readByte (VIDEO_AREA_SIZE - 1);
    }
    break;

  default:
    if (index > MEM_SIZE - 2) {
      throw RuntimeException ("%s:%d: index out of bounds.", __FILE__, __LINE__);
    }
    index -= VIDEO_AREA_SIZE;
    result = m_buffer[index + 1];
    result <<= 8;
    result |= m_buffer[index];
  }
  return result;
}

void Memory::write (Address address, int val, bool wide) {
  if (wide) {
    writeWord (address, val);
  } else {
    writeByte (address, val);
  }
}

void Memory::writeByte (Address address, int val) {
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
    m_buffer[index] = val;
    break;

  case 5:
    m_videoMemory.writeByte (index - VIDEO_AREA_START, val);
    break;

  default:
    m_buffer[index - VIDEO_AREA_SIZE] = val;
  }
}

void Memory::writeBytes (Address address, const uint8_t* src, uint16_t count) {
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
    memcpy (m_buffer + index, src, count);
    break;

  case 4:
    if (index + count <= VIDEO_AREA_START) {
      memcpy (m_buffer + index, src, count);
    } else {
      memcpy (m_buffer + index, src, VIDEO_AREA_START - index);
      m_videoMemory.writeBytes (0, src + VIDEO_AREA_START - index, count - (VIDEO_AREA_START - index));
    }
    break;

  case 5:
    if (index + count <= HIGH_AREA_START) {
      m_videoMemory.writeBytes (index - VIDEO_AREA_START, src, count);
    } else {
      m_videoMemory.writeBytes (index - VIDEO_AREA_START, src, HIGH_AREA_START - index);
      memcpy (m_buffer + VIDEO_AREA_START, src + HIGH_AREA_START - index, count - (HIGH_AREA_START - index));
    }
    break;

  default:
    if (index + count > MEM_SIZE) {
      throw RuntimeException ("%s:%d: index out of bounds.", __FILE__, __LINE__);
    }
    memcpy (m_buffer + index - VIDEO_AREA_SIZE, src, count);
  }
}

void Memory::writeWord (Address address, int val) {
  int index = address;
  switch (index >> 17) {
  case 0:
  case 1:
  case 2:
  case 3:
    m_buffer[index] = val;
    m_buffer[index + 1] = val >> 8;
    break;

  case 4:
    m_buffer[index] = val;
    if (index < VIDEO_AREA_START - 1) {
      m_buffer[index + 1] = val >> 8;
    } else {
      m_videoMemory.writeByte (0, val >> 8);
    }
    break;

  case 5:
    if (index < HIGH_AREA_START - 1) {
      m_videoMemory.writeWord (index - VIDEO_AREA_START, val);
    } else {
      m_videoMemory.writeByte (VIDEO_AREA_SIZE - 1, val);
      m_buffer[VIDEO_AREA_START] = val >> 8;
    }
    break;

  default:
    if (index > MEM_SIZE - 2) {
      throw RuntimeException ("%s:%d: index out of bounds.", __FILE__, __LINE__);
    }
    index -= VIDEO_AREA_SIZE;
    m_buffer[index] = val;
    m_buffer[index + 1] = val >> 8;
  }
}
