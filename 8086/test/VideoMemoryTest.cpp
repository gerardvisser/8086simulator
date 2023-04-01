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

void test::readByte_memoryMap (void) {
  printf ("test::readByte_memoryMap: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0x00000201;
  buffer[0x7FFF] = 0x00000403;
  buffer[0x8000] = 0x00000605;
  buffer[0xFFFF] = 0x00000807;
  videoMem.readMode (0);
  videoMem.memoryMode (1);

  videoMem.memoryMapSelect (1); // 00000 - 0FFFF
  int byte = videoMem.readByte (0);
  if (byte != 1) {
    printf ("Error reading byte 0 for memory map 1\n");
    return;
  }
  byte = videoMem.readByte (0x7FFF);
  if (byte != 3) {
    printf ("Error reading byte 0x7FFF for memory map 1\n");
    return;
  }
  byte = videoMem.readByte (0x8000);
  if (byte != 5) {
    printf ("Error reading byte 0x8000 memory map 1\n");
    return;
  }
  byte = videoMem.readByte (0xFFFF);
  if (byte != 7) {
    printf ("Error reading byte 0xFFFF memory map 1\n");
    return;
  }
  byte = videoMem.readByte (0x10000);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x10000 memory map 1\n");
    return;
  }
  byte = videoMem.readByte (0x1FFFF);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x1FFFF memory map 1\n");
    return;
  }

  videoMem.memoryMapSelect (2); // 10000 - 17FFF
  byte = videoMem.readByte (0);
  if (byte != 0xFF) {
    printf ("Error reading byte 0 for memory map 2\n");
    return;
  }
  byte = videoMem.readByte (0xFFFF);
  if (byte != 0xFF) {
    printf ("Error reading byte 0xFFFF memory map 2\n");
    return;
  }
  byte = videoMem.readByte (0x10000);
  if (byte != 1) {
    printf ("Error reading byte 0x10000 memory map 2\n");
    return;
  }
  byte = videoMem.readByte (0x17FFF);
  if (byte != 3) {
    printf ("Error reading byte 0x17FFF for memory map 2\n");
    return;
  }
  byte = videoMem.readByte (0x18000);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x18000 memory map 2\n");
    return;
  }
  byte = videoMem.readByte (0x1FFFF);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x1FFFF memory map 2\n");
    return;
  }

  videoMem.memoryMapSelect (3); // 18000 - 1FFFF
  byte = videoMem.readByte (0);
  if (byte != 0xFF) {
    printf ("Error reading byte 0 for memory map 3\n");
    return;
  }
  byte = videoMem.readByte (0x10000);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x10000 memory map 3\n");
    return;
  }
  byte = videoMem.readByte (0x17FFF);
  if (byte != 0xFF) {
    printf ("Error reading byte 0x17FFF for memory map 3\n");
    return;
  }
  byte = videoMem.readByte (0x18000);
  if (byte != 1) {
    printf ("Error reading byte 0x18000 memory map 3\n");
    return;
  }
  byte = videoMem.readByte (0x1FFFF);
  if (byte != 3) {
    printf ("Error reading byte 0x1FFFF memory map 2\n");
    return;
  }

  videoMem.memoryMode (0); // Odd/Even
  videoMem.memoryMapSelect (0); // 00000 - 1FFFF
  byte = videoMem.readByte (0);
  if (byte != 1) {
    printf ("Error reading byte 0 for memory map 0\n");
    return;
  }
  byte = videoMem.readByte (1);
  if (byte != 2) {
    printf ("Error reading byte 1 for memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0xFFFE);
  if (byte != 3) {
    printf ("Error reading byte 0xFFFE for memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0xFFFF);
  if (byte != 4) {
    printf ("Error reading byte 0xFFFF memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0x10000);
  if (byte != 5) {
    printf ("Error reading byte 0x10000 memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0x10001);
  if (byte != 6) {
    printf ("Error reading byte 0x10001 memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0x1FFFE);
  if (byte != 7) {
    printf ("Error reading byte 0x1FFFE memory map 0\n");
    return;
  }
  byte = videoMem.readByte (0x1FFFF);
  if (byte != 8) {
    printf ("Error reading byte 0x1FFFF memory map 0\n");
    return;
  }
  printf ("Ok\n");
}

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

static uint32_t createBytePattern (uint8_t value, int pattern) {
  uint32_t result = 0;
  int mask = 8;
  for (int i = 0; i < 4; ++i) {
    result <<= 8;
    if ((pattern & mask) != 0) {
      result |= value;
    }
    mask >>= 1;
  }
  return result;
}

void test::writeByte_planar_mode0 (void) {
  printf ("test::writeByte_planar_mode0: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.writeMode (0);

  videoMem.writeByte (0, 0x25);
  if (buffer[0] != 0x25252525) {
    printf ("Error writing byte with rotation=0, enable set/reset=0000, no logical op, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (2);
  videoMem.writeByte (1, 0x25);
  if (buffer[1] != 0x49494949) {
    printf ("Error writing byte with rotation=2, enable set/reset=0000, no logical op, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.enableSetReset (0xA);
  videoMem.setReset (0xC);
  videoMem.writeByte (2, 0x25);
  if (buffer[2] != 0xFF490049) {
    printf ("Error writing byte with rotation=2, enable set/reset=1010, set/reset=1100, no logical op, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  buffer[0x1000] = 0x55555555;
  videoMem.readByte (0x1000); // Loads the latch register.

  videoMem.enableSetReset (0x5);
  videoMem.rotateCount (3);
  videoMem.writeByte (3, 0x99);
  if (buffer[3] != 0x33FF3300) {
    printf ("Error writing byte with rotation=3, enable set/reset=0101, set/reset=1100, no logical op, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (1);
  videoMem.logicalOperation (1);
  videoMem.writeByte (4, 0x66);
  if (buffer[4] != 0x11551100) {
    printf ("Error writing byte with rotation=1, enable set/reset=0101, set/reset=1100, logical op=AND, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (4);
  videoMem.logicalOperation (2);
  videoMem.writeByte (5, 0x33);
  if (buffer[5] != 0x77FF7755) {
    printf ("Error writing byte with rotation=4, enable set/reset=0101, set/reset=1100, logical op=OR, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (5);
  videoMem.logicalOperation (3);
  videoMem.writeByte (6, 0x66);
  if (buffer[6] != 0x66AA6655) {
    printf ("Error writing byte with rotation=5, enable set/reset=0101, set/reset=1100, logical op=XOR, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (6);
  videoMem.bitMask (0xA7);
  videoMem.writeByte (7, 0xCC);
  if (buffer[7] != 0x76F27655) {
    printf ("Error writing byte with rotation=6, enable set/reset=0101, set/reset=1100, logical op=XOR, bit mask=10100111, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (7);
  videoMem.writePlaneEnable (0x7);
  videoMem.writeByte (8, 0x99);
  if (buffer[8] != 0x00F27655) {
    printf ("Error writing byte with rotation=7, enable set/reset=0101, set/reset=1100, logical op=XOR, bit mask=10100111, write plane enable=0111\n");
    return;
  }


  videoMem.rotateCount (0);
  videoMem.logicalOperation (0);
  videoMem.bitMask (0xFF);
  videoMem.writePlaneEnable (0xF);

  for (int sr = 0; sr < 16; ++sr) {
    videoMem.setReset (sr);
    int setReset = createBytePattern (0xFF, sr);
    for (int esr = 0; esr < 16; ++esr) {
      videoMem.enableSetReset (esr);
      int expected = createBytePattern (0x52, esr ^ 0xF);
      expected |= setReset & createBytePattern (0xFF, esr);
      videoMem.writeByte (9, 0x52);
      if (buffer[9] != expected) {
        printf ("Error writing byte with enable set/reset=0x%X, set/reset=0x%X\n", esr, sr);
        return;
      }
    }
  }

  videoMem.enableSetReset (0);
  videoMem.readByte (0x1001); // Loads the latch register.
  for (int bm = 0; bm < 256; ++bm) {
    int expected = createBytePattern (bm, 0xF);
    videoMem.bitMask (bm);
    videoMem.writeByte (10, 0xFF);
    if (buffer[10] != expected) {
      printf ("Error writing byte with bit mask=0x%02X\n", bm);
      return;
    }
  }

  for (int wpe = 0; wpe < 16; ++wpe) {
    int expected = createBytePattern (0xFF, wpe);
    videoMem.writePlaneEnable (wpe);
    videoMem.writeByte (11 + wpe, 0xFF);
    if (buffer[11 + wpe] != expected) {
      printf ("Error writing byte with write plane enable=0x%X\n", wpe);
      return;
    }
  }

  printf ("Ok\n");
}
