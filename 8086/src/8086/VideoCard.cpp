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

#include <8086/VideoCard.h>
#include "dacData.h"
#include "font.h"

VideoCard::VideoCard (Renderer& renderer) : m_videoOutputController (renderer, m_videoMemory) {
  m_graphicsRegisterIndex = 0;
  m_sequencerRegisterIndex = 0;
  m_videoOutputThread = std::move (std::thread (std::ref (m_videoOutputController)));
}

VideoCard::~VideoCard (void) {
  m_videoOutputController.stopVideoOutputThread ();
  if (m_videoOutputThread.joinable ()) {
    m_videoOutputThread.join ();
  }
}

/*
The data is loaded in segment 0xC000.
Offset 0 - 0x10 are reserved for (word) variables:
0: dacdata offset CGA colours.
2: dacdata offset EGA colours.
4: dacdata offset default 256 colours.
6: 8x8 font
8: 8x14 font
A: 8x16 font
*/
void VideoCard::loadRom (Memory& memory) {
  Address variableAddress (0xC000, 0);
  Address dataAddress (0xC000, 0x10);

  memory.writeWord (variableAddress, dataAddress.offset);
  variableAddress.offset += 2;
  memory.writeWord (variableAddress, dataAddress.offset + 0xC0);
  variableAddress.offset += 2;
  memory.writeWord (variableAddress, dataAddress.offset + 0x180);
  variableAddress.offset += 2;
  memory.writeBytes (dataAddress, rom::dacData, 1152);
  dataAddress.offset += 1152;

  memory.writeWord (variableAddress, dataAddress.offset);
  variableAddress.offset += 2;
  memory.writeWord (variableAddress, dataAddress.offset + 0x800);
  variableAddress.offset += 2;
  memory.writeWord (variableAddress, dataAddress.offset + 0x1600);
  variableAddress.offset += 2;
  memory.writeBytes (dataAddress, rom::font, 9728);
}

int VideoCard::readByte (uint16_t port) {
  switch (port) {
  case 0x3C4:
    return m_sequencerRegisterIndex;

  case 0x3C5:
    return readSequencerRegister ();

  case 0x3CE:
    return m_graphicsRegisterIndex;

  case 0x3CF:
    return readGraphicsRegister ();
  }
  return 0xFF;
}

VideoMemory& VideoCard::videoMemory (void) {
  return m_videoMemory;
}

void VideoCard::writeByte (uint16_t port, uint8_t value) {
  switch (port) {
  case 0x3C4:
    m_sequencerRegisterIndex = value;
    break;

  case 0x3C5:
    writeSequencerRegister (value);
    break;

  case 0x3CE:
    m_graphicsRegisterIndex = value;
    break;

  case 0x3CF:
    writeGraphicsRegister (value);
    break;
  }
}

int VideoCard::readGraphicsRegister (void) const {
  int value;
  switch (m_graphicsRegisterIndex) {
  case 0:
    return m_videoMemory.setReset ();

  case 1:
    return m_videoMemory.enableSetReset ();

  case 2:
    return m_videoMemory.colourCompare ();

  case 3:
    value = m_videoMemory.logicalOperation ();
    value <<= 3;
    value |= m_videoMemory.rotateCount ();
    return value;

  case 4:
    return m_videoMemory.readPlaneSelect ();

  case 5:
    value = m_videoMemory.shiftMode ();
    value <<= 1;
    value |= m_videoMemory.memoryMode () & 1 ^ 1;
    value <<= 1;
    value |= m_videoMemory.readMode ();
    value <<= 3;
    value |= m_videoMemory.writeMode ();
    return value;

  case 6:
    value = m_videoMemory.memoryMapSelect ();
    value <<= 1;
    value |= m_videoMemory.memoryMode () & 1 ^ 1;
    value <<= 1;
    value |= m_videoMemory.graphicsMode ();
    return value;

  case 7:
    return m_videoMemory.colourDontCare ();

  case 8:
    return m_videoMemory.bitMask ();
  }
  return 0xFF;
}

int VideoCard::readSequencerRegister (void) const {
  int value;
  switch (m_sequencerRegisterIndex) {
    /*
  case 1:
    //bit 0 altijd 1.
    */

  case 2:
    return m_videoMemory.writePlaneEnable ();

  case 4:
    value = m_videoMemory.memoryMode ();
    value <<= 1;
    value |= 1;
    value <<= 1;
    return value;
  }
  return 0xFF;
}

void VideoCard::writeGraphicsRegister (uint8_t value) {
  switch (m_graphicsRegisterIndex) {
  case 0:
    m_videoMemory.setReset (value);
    break;

  case 1:
    m_videoMemory.enableSetReset (value);
    break;

  case 2:
    m_videoMemory.colourCompare (value);
    break;

  case 3:
    m_videoMemory.rotateCount (value);
    m_videoMemory.logicalOperation (value >> 3);
    break;

  case 4:
    m_videoMemory.readPlaneSelect (value);
    break;

  case 5:
    m_videoMemory.writeMode (value);
    m_videoMemory.readMode (value >> 3);
    m_videoMemory.shiftMode (value >> 5);
    break;

  case 6:
    m_videoMemory.graphicsMode (value & 1);
    m_videoMemory.memoryMapSelect (value >> 2);
    break;

  case 7:
    m_videoMemory.colourDontCare (value);
    break;

  case 8:
    m_videoMemory.bitMask (value);
    break;
  }
}

void VideoCard::writeSequencerRegister (uint8_t value) {
  switch (m_sequencerRegisterIndex) {
    /*
  case 1:
    break;
    */

  case 2:
    m_videoMemory.writePlaneEnable (value);
    break;

  case 4:
    m_videoMemory.memoryMode (value >> 2);
    break;
  }
}
