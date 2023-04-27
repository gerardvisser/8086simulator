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

#ifndef __VIDEO_CARD_INCLUDED
#define __VIDEO_CARD_INCLUDED

#include <thread>
#include <8086/Memory.h>
#include <8086/VideoOutputController.h>

class VideoCard {
private:
  VideoMemory m_videoMemory;
  VideoOutputController m_videoOutputController;
  std::thread m_videoOutputThread;
  int m_graphicsRegisterIndex;
  int m_sequencerRegisterIndex;
  int m_attributeRegisterIndex;
  bool m_attributeDataWrite;
  uint8_t m_dacReadIndex;
  uint8_t m_dacWriteIndex;
  uint8_t m_dacState;
  uint8_t m_dacCycleIndex;
  Colour m_colour;

public:
  explicit VideoCard (Renderer& renderer);

  VideoCard (const VideoCard&) = delete;
  VideoCard (VideoCard&&) = delete;

  ~VideoCard (void);

  VideoCard& operator= (const VideoCard&) = delete;
  VideoCard& operator= (VideoCard&&) = delete;

  void loadRom (Memory& memory);
  int readByte (uint16_t port);
  void writeByte (uint16_t port, uint8_t value);

  VideoMemory& videoMemory (void);

private:
  int readAttributeRegister (void) const;
  int readDacValue (void);
  int readGraphicsRegister (void) const;
  int readSequencerRegister (void) const;
  void writeAttributeRegister (uint8_t value);
  void writeDacValue (uint8_t value);
  void writeGraphicsRegister (uint8_t value);
  void writeSequencerRegister (uint8_t value);
};

#endif
