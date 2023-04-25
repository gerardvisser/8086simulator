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
#include <thread>
#include <8086/VideoOutputController.h>

using namespace std::chrono_literals;

VideoOutputController::VideoOutputController (Renderer& renderer, VideoMemory& videoMemory) : m_renderer (renderer), m_videoMemory (videoMemory) {
  m_dac = new Colour[0x100];
  m_pixels = (uint8_t*) malloc (0x80000);
  m_videoOutputThreadToBeStopped = false;
  m_screenDisabled = true;
  m_horizontalEnd = 0;
  m_verticalEnd = 0;
  m_overflowRegister = 0;
  m_colourSelect = 0;
  m_scanDoubling = false;
  m_widePixels = false;
  m_allColourSelectBitsEnabled = false;

  for (int i = 0; i < 16; ++i) {
    m_paletteRegisters[i] = 0;
  }
}

VideoOutputController::~VideoOutputController (void) {
  free (m_pixels);
  delete[] m_dac;
}

void VideoOutputController::operator() (void) {
  while (!m_videoOutputThreadToBeStopped) {
    drawScreen ();
    std::this_thread::sleep_for (250ms);
  }
}

void VideoOutputController::drawScreen (void) {
  std::lock_guard<std::mutex> lock (m_mutex);
  if (!m_screenDisabled) {
    int width = m_horizontalEnd + 1;
    int height = heightInScanLines ();
    if (m_videoMemory.getPixels (m_pixels, width, height)) {

      m_renderer.setDrawColour (Colour::BLACK);
      m_renderer.clear ();

      width <<= 3 - m_widePixels; /* width in pixels */
      m_renderer.setResolution (width, height);
      if (m_widePixels) {
        for (int h = 0; h < height; ++h) {
          for (int w = 0; w < width; ++w) {
            int index = 2 * (width * h + w);
            int colourIndex = m_pixels[index] << 4;
            colourIndex |= m_pixels[index + 1];
            m_renderer.setDrawColour (m_dac[colourIndex]);
            m_renderer.drawPoint (w, h);
          }
        }
      } else {
        int highBits = m_colourSelect << 4;
        int mask;
        if (m_allColourSelectBitsEnabled) {
          mask = 0x0F;
        } else {
          mask = 0x3F;
          highBits &= 0xC0;
        }
        for (int h = 0; h < height; ++h) {
          for (int w = 0; w < width; ++w) {
            int colourIndex = m_paletteRegisters[m_pixels[width * h + w]];
            colourIndex &= mask;
            colourIndex |= highBits;
            m_renderer.setDrawColour (m_dac[colourIndex]);
            m_renderer.drawPoint (w, h);
          }
        }
      }
      m_renderer.present ();

    }
  }
}

int VideoOutputController::heightInScanLines (void) {
  int result = m_overflowRegister >> 6 & 1;
  result <<= 1;
  result |= m_overflowRegister >> 1 & 1;
  result <<= 8;
  result |= m_verticalEnd;
  result += 1;
  result >>= m_scanDoubling;
  return result;
}

void VideoOutputController::stopVideoOutputThread (void) {
  m_videoOutputThreadToBeStopped = true;
}

/* 3C0, 10, bit: 7 */
bool VideoOutputController::allColourSelectBitsEnabled (void) const {
  return m_allColourSelectBitsEnabled;
}

void VideoOutputController::allColourSelectBitsEnabled (bool val) {
  m_allColourSelectBitsEnabled = val;
}

/* 3C0, 14 */
uint8_t VideoOutputController::colourSelect (void) const {
  return m_colourSelect;
}

void VideoOutputController::colourSelect (uint8_t val) {
  m_colourSelect = val & 0xF;
}

/* 3D4, 1 */
uint8_t VideoOutputController::horizontalEnd (void) const {
  return m_horizontalEnd;
}

void VideoOutputController::horizontalEnd (uint8_t val) {
  m_horizontalEnd = val;
}

/* 3D4, 7 */
uint8_t VideoOutputController::overflowRegister (void) const {
  return m_overflowRegister;
}

void VideoOutputController::overflowRegister (uint8_t val) {
  m_overflowRegister = val;
}

/* 3D4, 9, bit: 7 */
bool VideoOutputController::scanDoubling (void) const {
  return m_scanDoubling;
}

void VideoOutputController::scanDoubling (bool val) {
  m_scanDoubling = val;
}

/* 3C4, 1, bit: 5 */
bool VideoOutputController::screenDisabled (void) const {
  return m_screenDisabled;
}

void VideoOutputController::screenDisabled (bool val) {
  if (val != m_screenDisabled) {
    if (val) {
      std::lock_guard<std::mutex> lock (m_mutex);
      m_renderer.setDrawColour (Colour::BLACK);
      m_renderer.clear ();
      m_renderer.present ();
      m_screenDisabled = true;
    } else {
      m_screenDisabled = false;
    }
  }
}

/* 3D4, 12 */
uint8_t VideoOutputController::verticalEnd (void) const {
  return m_verticalEnd;
}

void VideoOutputController::verticalEnd (uint8_t val) {
  m_verticalEnd = val;
}

/* 3C0, 10, bit: 6 */
bool VideoOutputController::widePixels (void) const {
  return m_widePixels;
}

void VideoOutputController::widePixels (bool val) {
  m_widePixels = val;
}
