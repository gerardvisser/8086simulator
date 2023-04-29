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

static constexpr double DAC_6_BITS_TO_8_BITS = 255.0 / 63;
static constexpr double DAC_8_BITS_TO_6_BITS = 63.0 / 255;

VideoCard::VideoCard (Renderer& renderer) : m_videoOutputController (renderer, m_videoMemory) {
  m_graphicsRegisterIndex = 0;
  m_sequencerRegisterIndex = 0;
  m_crtRegisterIndex = 0;
  m_attributeRegisterIndex = 0;
  m_attributeDataWrite = false;
  m_dacReadIndex = 0;
  m_dacWriteIndex = 0;
  m_dacState = 3;
  m_dacCycleIndex = 0;
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
6: 8x8 font offset
8: 8x14 font offset
A: 8x16 font offset
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
  case 0x3C0:
    return m_attributeRegisterIndex | m_videoOutputController.paletteAddressSource () << 5;

  case 0x3C1:
    return readAttributeRegister ();

  case 0x3C4:
    return m_sequencerRegisterIndex;

  case 0x3C5:
    return readSequencerRegister ();

  case 0x3C7:
    return m_dacState;

  case 0x3C8:
    return m_dacWriteIndex;

  case 0x3C9:
    return readDacValue ();

  case 0x3CE:
    return m_graphicsRegisterIndex;

  case 0x3CF:
    return readGraphicsRegister ();

  case 0x3D4:
    return m_crtRegisterIndex;

  case 0x3D5:
    return readCrtRegister ();

  case 0x3DA:
    m_attributeDataWrite = false;
    return 0;
  }
  return 0xFF;
}

VideoMemory& VideoCard::videoMemory (void) {
  return m_videoMemory;
}

void VideoCard::writeByte (uint16_t port, uint8_t value) {
  switch (port) {
  case 0x3C0:
    if (m_attributeDataWrite) {
      writeAttributeRegister (value);
    } else {
      m_attributeRegisterIndex = value & 0x1F;
      m_videoOutputController.paletteAddressSource ((value & 0x20) != 0);
    }
    m_attributeDataWrite = !m_attributeDataWrite;
    break;

  case 0x3C4:
    m_sequencerRegisterIndex = value;
    break;

  case 0x3C5:
    writeSequencerRegister (value);
    break;

  case 0x3C7:
    m_dacState = 3;
    m_dacCycleIndex = 0;
    m_dacReadIndex = value;
    m_colour = m_videoOutputController.dacColour (m_dacReadIndex);
    break;

  case 0x3C8:
    m_dacState = 0;
    m_dacCycleIndex = 0;
    m_dacWriteIndex = value;
    break;

  case 0x3C9:
    writeDacValue (value);
    break;

  case 0x3CE:
    m_graphicsRegisterIndex = value;
    break;

  case 0x3CF:
    writeGraphicsRegister (value);
    break;

  case 0x3D4:
    m_crtRegisterIndex = value;
    break;

  case 0x3D5:
    writeCrtRegister (value);
    break;
  }
}

void VideoCard::writeWord (uint16_t port, int value) {
  writeByte (port, value);
  writeByte (port + 1, value >> 8);
}

int VideoCard::readAttributeRegister (void) const {
  if (m_attributeRegisterIndex < 0x10) {
    return m_videoOutputController.paletteRegister (m_attributeRegisterIndex);
  } else if (m_attributeRegisterIndex == 0x10) {
    int value = m_videoOutputController.allColourSelectBitsEnabled ();
    value <<= 1;
    value |= m_videoOutputController.widePixels ();
    value <<= 6;
    value |= m_videoMemory.graphicsMode ();
    /* Bit 2: Line graphics enable to be implemented later.  */
    return value;
  } else if (m_attributeRegisterIndex == 0x14) {
    return m_videoOutputController.colourSelect ();
  }
  return 0xFF;
}

int VideoCard::readCrtRegister (void) const {
  int value;
  switch (m_crtRegisterIndex) {
  case 1:
    return m_videoOutputController.horizontalEnd ();

  case 7:
    return m_videoOutputController.overflowRegister ();

  case 9:
    value = m_videoOutputController.scanDoubling ();
    value <<= 1;
    value |= 1;
    value <<= 6;
    /* Bits 4 - 0: maximum scan line.  */
    return value;

  case 0x12:
    return m_videoOutputController.verticalEnd ();

  case 0x17:
    return m_videoMemory.cgaAddressing () ^ 1;
  }
  return 0xFF;
}

int VideoCard::readDacValue (void) {
  int value;
  switch (m_dacCycleIndex) {
  case 0:
    value = (int) (DAC_8_BITS_TO_6_BITS * m_colour.red + 0.5);
    break;

  case 1:
    value = (int) (DAC_8_BITS_TO_6_BITS * m_colour.green + 0.5);
    break;

  case 2:
    value = (int) (DAC_8_BITS_TO_6_BITS * m_colour.blue + 0.5);
  }
  ++m_dacCycleIndex;
  if (m_dacCycleIndex == 3) {
    ++m_dacReadIndex;
    m_dacCycleIndex = 0;
    m_colour = m_videoOutputController.dacColour (m_dacReadIndex);
  }
  return value;
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
  case 1:
    value = m_videoOutputController.screenOff () << 5;
    value |= 1; /* bit 0: 9/8 dot mode, to be implemented later.  */
    return value;

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

void VideoCard::writeAttributeRegister (uint8_t value) {
  if (m_attributeRegisterIndex < 0x10) {
    m_videoOutputController.paletteRegister (m_attributeRegisterIndex, value);
  } else if (m_attributeRegisterIndex == 0x10) {
    m_videoOutputController.allColourSelectBitsEnabled ((value & 0x80) != 0);
    m_videoOutputController.widePixels ((value & 0x40) != 0);
  } else if (m_attributeRegisterIndex == 0x14) {
    m_videoOutputController.colourSelect (value);
  }
}

void VideoCard::writeCrtRegister (uint8_t value) {
  switch (m_crtRegisterIndex) {
  case 1:
    m_videoOutputController.horizontalEnd (value);
    break;

  case 7:
    m_videoOutputController.overflowRegister (value);
    break;

  case 9:
    m_videoOutputController.scanDoubling ((value & 0x80) != 0);
    /* Bits 4 - 0: maximum scan line.  */
    break;

  case 0x12:
    m_videoOutputController.verticalEnd (value);
    break;

  case 0x17:
    m_videoMemory.cgaAddressing (value & 1 ^ 1);
    break;
  }
}

void VideoCard::writeDacValue (uint8_t value) {
  value &= 0x3F;
  switch (m_dacCycleIndex) {
  case 0:
    m_colour.red = (int) (DAC_6_BITS_TO_8_BITS * value + 0.5);
    break;

  case 1:
    m_colour.green = (int) (DAC_6_BITS_TO_8_BITS * value + 0.5);
    break;

  case 2:
    m_colour.blue = (int) (DAC_6_BITS_TO_8_BITS * value + 0.5);
  }
  ++m_dacCycleIndex;
  if (m_dacCycleIndex == 3) {
    m_videoOutputController.dacColour (m_dacWriteIndex, m_colour);
    m_dacCycleIndex = 0;
    ++m_dacWriteIndex;
  }
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
  case 1:
    m_videoOutputController.screenOff ((value & 0x20) != 0);
    /* bit 0: 9/8 dot mode, to be implemented later.  */
    break;

  case 2:
    m_videoMemory.writePlaneEnable (value);
    break;

  case 4:
    m_videoMemory.memoryMode (value >> 2);
    break;
  }
}
