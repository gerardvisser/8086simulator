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

#ifndef __VIDEO_OUTPUT_CONTROLLER_INCLUDED
#define __VIDEO_OUTPUT_CONTROLLER_INCLUDED

#include <mutex>
#include <8086/VideoMemory.h>
#include <8086/sdl/Renderer.h>

class VideoOutputController {
private:
  std::mutex m_mutex;
  VideoMemory& m_videoMemory;
  Renderer& m_renderer;
  uint8_t* m_pixels;
  Colour* m_dac;
  uint8_t m_paletteRegisters[16];
  bool m_videoOutputThreadToBeStopped;
  bool m_screenCleared;
  bool m_screenDisabled;
  uint8_t m_horizontalEnd;
  uint8_t m_verticalEnd;
  uint8_t m_overflowRegister;
  uint8_t m_colourSelect;
  bool m_scanDoubling;
  bool m_widePixels;
  bool m_allColourSelectBitsEnabled;

public:
  VideoOutputController (Renderer& renderer, VideoMemory& videoMemory);

  VideoOutputController (const VideoOutputController&) = delete;
  VideoOutputController (VideoOutputController&&) = delete;

  ~VideoOutputController (void);

  VideoOutputController& operator= (const VideoOutputController&) = delete;
  VideoOutputController& operator= (VideoOutputController&&) = delete;

  void operator() (void);

  void stopVideoOutputThread (void);

  bool allColourSelectBitsEnabled (void) const;
  void allColourSelectBitsEnabled (bool val);
  uint8_t colourSelect (void) const;
  void colourSelect (uint8_t val);
  uint8_t horizontalEnd (void) const;
  void horizontalEnd (uint8_t val);
  uint8_t overflowRegister (void) const;
  void overflowRegister (uint8_t val);
  bool scanDoubling (void) const;
  void scanDoubling (bool val);
  bool screenDisabled (void) const;
  void screenDisabled (bool val);
  uint8_t verticalEnd (void) const;
  void verticalEnd (uint8_t val);
  bool widePixels (void) const;
  void widePixels (bool val);

private:
  void drawScreen (void);
  int heightInScanLines (void);
};

#endif
