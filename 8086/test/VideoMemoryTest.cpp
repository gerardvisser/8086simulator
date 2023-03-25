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

#include "VideoMemoryTest.h"
#include <cstdio>
#include <8086/VideoMemory.h>

/*
Plane 3: 1 1 1 1 0 0 0 1 - 0 0 0 1 0 0 1 0
Plane 2: 1 1 1 0 0 0 1 0 - 0 0 1 1 0 1 0 0
Plane 1: 1 1 0 1 0 0 1 1 - 0 1 0 1 0 1 1 0
Plane 0: 1 1 0 0 0 1 0 0 - 0 1 1 1 1 0 0 0
*/
void test::readByte_planar_mode0 (void) {
  printf ("test::readByte_planar_mode0: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0xF1E2D3C4;
  buffer[1] = 0x12345678;
  videoMem.readMode (0);

  int expectedByte = 0xC4;
  for(int plane = 0; plane < 4; ++plane) {
    videoMem.readPlaneSelect (plane);
    if (videoMem.readByte (0) != expectedByte) {
      printf ("Error reading plane %d at offset 0\n", plane);
      return;
    }
    expectedByte += 0xF;
  }

  expectedByte = 0x78;
  for(int plane = 0; plane < 4; ++plane) {
    videoMem.readPlaneSelect (plane);
    if (videoMem.readByte (1) != expectedByte) {
      printf ("Error reading plane %d at offset 1\n", plane);
      return;
    }
    expectedByte -= 0x22;
  }
  printf ("Ok\n");
}

static int compareColour (int pixel, int colourCompare, int colourDontCare) {
  pixel &= colourDontCare;
  colourCompare &= colourDontCare;
  return pixel == colourCompare;
}

/*
Plane 3: 0 0 0 0 0 0 0 0 - 1 1 1 1 1 1 1 1
Plane 2: 0 0 0 0 1 1 1 1 - 0 0 0 0 1 1 1 1
Plane 1: 0 0 1 1 0 0 1 1 - 0 0 1 1 0 0 1 1
Plane 0: 0 1 0 1 0 1 0 1 - 0 1 0 1 0 1 0 1
*/
void test::readByte_planar_mode1 (void) {
  printf ("test::readByte_planar_mode1: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0x000F3355;
  buffer[1] = 0xFF0F3355;
  videoMem.readMode (1);

  for (int colourCompare = 0; colourCompare < 16; ++colourCompare) {
    videoMem.colourCompare (colourCompare);
    for (int colourDontCare = 0; colourDontCare < 16; ++colourDontCare) {
      videoMem.colourDontCare (colourDontCare);
      uint8_t expectedByte = 0;
      for (int pixel = 0; pixel < 8; ++pixel) {
        expectedByte <<= 1;
        expectedByte |= compareColour (pixel, colourCompare, colourDontCare);
      }
      if (videoMem.readByte (0) != expectedByte) {
        printf ("Error reading byte 0 with colourCompare=%d and colourDontCare=%d\n", colourCompare, colourDontCare);
        return;
      }

      for (int pixel = 8; pixel < 16; ++pixel) {
        expectedByte <<= 1;
        expectedByte |= compareColour (pixel, colourCompare, colourDontCare);
      }
      if (videoMem.readByte (1) != expectedByte) {
        printf ("Error reading byte 1 with colourCompare=%d and colourDontCare=%d\n", colourCompare, colourDontCare);
        return;
      }
    }
  }
  printf ("Ok\n");
}
