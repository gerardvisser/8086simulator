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

#define ODD_EVEN 0
#define PLANAR   1
#define CHAIN_4  3

void test::readByte_chain4 (void) {
  printf ("test::readByte_chain4: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0x04030201;
  buffer[1] = 0x08070605;
  videoMem.memoryMode (CHAIN_4);

  for (int i = 0; i < 8; ++i) {
    if (videoMem.readByte (i) != i + 1) {
      printf ("Error reading byte %d\n", i);
      return;
    }
  }
  printf ("Ok\n");
}

void test::readByte_memoryMap (void) {
  printf ("test::readByte_memoryMap: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0x00000201;
  buffer[0x7FFF] = 0x00000403;
  buffer[0x8000] = 0x00000605;
  buffer[0xFFFF] = 0x00000807;
  videoMem.readMode (0);
  videoMem.memoryMode (PLANAR);

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

  videoMem.memoryMode (ODD_EVEN);
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

void test::readByte_oddEven (void) {
  printf ("test::readByte_oddEven: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  buffer[0] = 0x06050201;
  buffer[1] = 0x08070403;
  videoMem.memoryMode (ODD_EVEN);

  videoMem.readPlaneSelect (0);
  for (int i = 0; i < 4; ++i) {
    if (videoMem.readByte (i) != i + 1) {
      printf ("Error reading byte %d, planes 0, 1\n", i);
      return;
    }
  }

  videoMem.readPlaneSelect (2);
  for (int i = 0; i < 4; ++i) {
    if (videoMem.readByte (i) != i + 5) {
      printf ("Error reading byte %d, planes 2, 3\n", i);
      return;
    }
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

void test::writeByte_chain4 (void) {
  printf ("test::writeByte_chain4: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.memoryMode (CHAIN_4);

  for (int i = 0; i < 8; ++i) {
    videoMem.writeByte (i, 0xF1 - 15 * i);
  }
  if (buffer[0] != 0xC4D3E2F1) {
    printf ("Error at offset 0 (expected: 0xC4D3E2F1, actual: 0x%08X)\n", buffer[0]);
    return;
  }
  if (buffer[1] != 0x8897A6B5) {
    printf ("Error at offset 1 (expected: 0x8897A6B5, actual: 0x%08X)\n", buffer[1]);
    return;
  }
  printf ("Ok\n");
}

void test::writeByte_memoryMap (void) {
  printf ("test::writeByte_memoryMap: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.writeMode (0);
  videoMem.memoryMode (ODD_EVEN);

  videoMem.memoryMapSelect (1); // 00000 - 0FFFF
  videoMem.writeByte (0, 1);
  videoMem.writeByte (0xFFFF, 2);
  videoMem.writeByte (0x10000, 3);
  if (buffer[0] != 0x00010001) {
    printf ("Error writing byte 0 for memory map 1\n");
    return;
  }
  if (buffer[0x7FFF] != 0x02000200) {
    printf ("Error writing byte 0xFFFF for memory map 1\n");
    return;
  }
  if (buffer[0x8000] != 0) {
    printf ("Error writing byte 0x10000 for memory map 1\n");
    return;
  }

  videoMem.memoryMapSelect (2); // 10000 - 17FFF
  videoMem.writeByte (0, 4);
  if (buffer[0] != 0x00010001) {
    printf ("Error writing byte 0 for memory map 2\n");
    return;
  }
  videoMem.writeByte (0x10000, 4);
  videoMem.writeByte (0x17FFF, 5);
  videoMem.writeByte (0x18000, 6);
  if (buffer[0] != 0x00040004) {
    printf ("Error writing byte 0x10000 for memory map 2\n");
    return;
  }
  if (buffer[0x3FFF] != 0x05000500) {
    printf ("Error writing byte 0x17FFF for memory map 2\n");
    return;
  }
  if (buffer[0x4000] != 0) {
    printf ("Error writing byte 0x18000 for memory map 2\n");
    return;
  }

  videoMem.memoryMapSelect (3); // 18000 - 1FFFF
  videoMem.writeByte (0x18000, 7);
  videoMem.writeByte (0x1FFFF, 8);
  videoMem.writeByte (0x20000, 9);
  if (buffer[0] != 0x00070007) {
    printf ("Error writing byte 0x18000 for memory map 3\n");
    return;
  }
  if (buffer[0x3FFF] != 0x08000800) {
    printf ("Error writing byte 0x1FFFF for memory map 3\n");
    return;
  }
  if (buffer[0x4000] != 0) {
    printf ("Error writing byte 0x20000 for memory map 3\n");
    return;
  }

  videoMem.memoryMapSelect (0); // 00000 - 1FFFF
  videoMem.writeByte (0, 10);
  videoMem.writeByte (0xFFFF, 11);
  videoMem.writeByte (0x10000, 12);
  videoMem.writeByte (0x1FFFF, 13);
  if (buffer[0] != 0x000A000A) {
    printf ("Error writing byte 0 for memory map 0\n");
    return;
  }
  if (buffer[0x7FFF] != 0x0B000B00) {
    printf ("Error writing byte 0xFFFF for memory map 0\n");
    return;
  }
  if (buffer[0x8000] != 0x000C000C) {
    printf ("Error writing byte 0x10000 for memory map 0\n");
    return;
  }
  if (buffer[0xFFFF] != 0x0D000D00) {
    printf ("Error writing byte 0x1FFFF for memory map 0\n");
    return;
  }
  printf ("Ok\n");
}

void test::writeByte_oddEven (void) {
  printf ("test::writeByte_oddEven: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.memoryMode (ODD_EVEN);

  videoMem.writePlaneEnable (0x3);
  for (int i = 0; i < 8; ++i) {
    videoMem.writeByte (i, 0xF1 - 15 * i);
  }
  if (buffer[0] != 0x0000E2F1) {
    printf ("Error at offset 0 (expected: 0x0000E2F1, actual: 0x%08X)\n", buffer[0]);
    return;
  }
  if (buffer[1] != 0x0000C4D3) {
    printf ("Error at offset 1 (expected: 0x0000C4D3, actual: 0x%08X)\n", buffer[1]);
    return;
  }
  if (buffer[2] != 0x0000A6B5) {
    printf ("Error at offset 2 (expected: 0x0000A6B5, actual: 0x%08X)\n", buffer[2]);
    return;
  }
  if (buffer[3] != 0x00008897) {
    printf ("Error at offset 3 (expected: 0x00008897, actual: 0x%08X)\n", buffer[3]);
    return;
  }

  videoMem.writePlaneEnable (0xC);
  for (int i = 0; i < 8; ++i) {
    videoMem.writeByte (i, 0x12 + 0x21 * i);
  }
  if (buffer[0] != 0x3312E2F1) {
    printf ("Error at offset 0 (expected: 0x3312E2F1, actual: 0x%08X)\n", buffer[0]);
    return;
  }
  if (buffer[1] != 0x7554C4D3) {
    printf ("Error at offset 1 (expected: 0x7554C4D3, actual: 0x%08X)\n", buffer[1]);
    return;
  }
  if (buffer[2] != 0xB796A6B5) {
    printf ("Error at offset 2 (expected: 0xB796A6B5, actual: 0x%08X)\n", buffer[2]);
    return;
  }
  if (buffer[3] != 0xF9D88897) {
    printf ("Error at offset 3 (expected: 0xF9D88897, actual: 0x%08X)\n", buffer[3]);
    return;
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
    uint32_t setReset = createBytePattern (0xFF, sr);
    for (int esr = 0; esr < 16; ++esr) {
      videoMem.enableSetReset (esr);
      uint32_t expected = createBytePattern (0x52, esr ^ 0xF);
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
    uint32_t expected = createBytePattern (bm, 0xF);
    videoMem.bitMask (bm);
    videoMem.writeByte (10, 0xFF);
    if (buffer[10] != expected) {
      printf ("Error writing byte with bit mask=0x%02X\n", bm);
      return;
    }
  }

  for (int wpe = 0; wpe < 16; ++wpe) {
    uint32_t expected = createBytePattern (0xFF, wpe);
    videoMem.writePlaneEnable (wpe);
    videoMem.writeByte (11 + wpe, 0xFF);
    if (buffer[11 + wpe] != expected) {
      printf ("Error writing byte with write plane enable=0x%X\n", wpe);
      return;
    }
  }
  printf ("Ok\n");
}

void test::writeByte_planar_mode1 (void) {
  printf ("test::writeByte_planar_mode1: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.writeMode (1);

  buffer[0x1000] = 0x12345678;
  videoMem.readByte (0x1000); // Loads the latch register.

  videoMem.writeByte (0, 0xFF);
  if (buffer[0] != 0x12345678) {
    printf ("Error writing byte with write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (3);
  videoMem.setReset (0x6);
  videoMem.enableSetReset (0x5);
  videoMem.logicalOperation (2);
  videoMem.bitMask (0x5A);
  videoMem.writeByte (1, 0x01);
  if (buffer[1] != 0x12345678) {
    printf ("Error writing byte with write plane enable=1111, rotation=3, enable set/reset=0101, set/reset=0110, logical op=OR, bit mask=01011010\n");
    return;
  }

  videoMem.writePlaneEnable (0x6);
  videoMem.writeByte (2, 0x00);
  if (buffer[2] != 0x00345600) {
    printf ("Error writing byte with write plane enable=0110\n");
    return;
  }
  printf ("Ok\n");
}

void test::writeByte_planar_mode2 (void) {
  printf ("test::writeByte_planar_mode2: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.writeMode (2);

  for (int i = 0; i < 256; ++i) {
    uint32_t expected = createBytePattern (0xFF, i & 0xF);
    videoMem.writeByte (i, i);
    if (buffer[i] != expected) {
      printf ("Error writing byte %02X with no logical op, bit mask=11111111, write plane enable=1111\n", i);
      return;
    }
  }

  videoMem.setReset (0xA);
  videoMem.enableSetReset (0xF);
  videoMem.writeByte (0, 0x05);
  if (buffer[0] != 0x00FF00FF) {
    printf ("Error writing byte 05 with no logical op, bit mask=11111111, write plane enable=1111, enable set/reset=1111, set/reset=1010\n");
    return;
  }

  videoMem.rotateCount (1);
  videoMem.writeByte (1, 0x05);
  if (buffer[1] != 0x00FF00FF) {
    printf ("Error writing byte 05 with no logical op, bit mask=11111111, write plane enable=1111, enable set/reset=1111, set/reset=1010, rotation=1\n");
    return;
  }

  buffer[0x1000] = 0xBB40E64D;
  videoMem.readByte (0x1000); // Loads the latch register.

  videoMem.logicalOperation (1);
  videoMem.writeByte (256, 0x6);
  if (buffer[256] != 0x0040E600) {
    printf ("Error writing byte 06 with logical op=AND, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.logicalOperation (3);
  videoMem.writeByte (257, 0x6);
  if (buffer[257] != 0xBBBF194D) {
    printf ("Error writing byte 06 with logical op=XOR, bit mask=11111111, write plane enable=1111\n");
    return;
  }

  videoMem.bitMask (0x4B);
  videoMem.writeByte (258, 0x6);
  if (buffer[258] != 0xBB0BAD4D) {
    printf ("Error writing byte 06 with logical op=XOR, bit mask=01001011, write plane enable=1111\n");
    return;
  }

  buffer[259] = 0xA9876501;
  videoMem.writePlaneEnable (0x9);
  videoMem.writeByte (259, 0x6);
  if (buffer[259] != 0xBB87654D) {
    printf ("Error writing byte 06 with logical op=XOR, bit mask=01001011, write plane enable=1001\n");
    return;
  }
  printf ("Ok\n");
}

void test::writeByte_planar_mode3 (void) {
  printf ("test::writeByte_planar_mode3: ");

  VideoMemory videoMem;
  uint32_t* buffer = videoMem.buffer ();
  videoMem.writeMode (3);

  videoMem.enableSetReset (0);
  for (int i = 0; i < 16; ++i) {
    buffer[i] = 0x01020304;
    videoMem.setReset (i);
    videoMem.writeByte (i, 0xFF);
    uint32_t expected = createBytePattern (0xFF, i);
    if (buffer[i] != expected) {
      printf ("Error writing byte with rotation=0, set/reset=0x%X, host byte=FF, bit mask=11111111, write plane enable=1111\n", i);
      return;
    }
  }

  buffer[0x1000] = 0xFF00FF00;
  videoMem.readByte (0x1000); // Loads the latch register.

  videoMem.setReset (0x5);
  for (int i = 0; i < 256; ++i) {
    videoMem.writeByte (i + 16, i);
    uint32_t expected = createBytePattern (i, 0x5);
    expected |= createBytePattern (i ^ 0xFF, 0xA);
    if (buffer[i + 16] != expected) {
      printf ("Error writing byte with rotation=0, set/reset=0101, host byte=0x%02X, bit mask=11111111, write plane enable=1111\n", i);
      return;
    }
  }

  videoMem.bitMask (0x33);
  videoMem.writeByte (0, 0x5C);
  if (buffer[0] != 0xEF10EF10) {
    printf ("Error writing byte with rotation=0, set/reset=0101, host byte=5C, bit mask=00110011, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (1);
  videoMem.writeByte (1, 0x5C);
  if (buffer[1] != 0xDD22DD22) {
    printf ("Error writing byte with rotation=1, set/reset=0101, host byte=5C, bit mask=00110011, write plane enable=1111\n");
    return;
  }

  videoMem.rotateCount (2);
  videoMem.writeByte (2, 0x5C);
  if (buffer[2] != 0xEC13EC13) {
    printf ("Error writing byte with rotation=2, set/reset=0101, host byte=5C, bit mask=00110011, write plane enable=1111\n");
    return;
  }

  videoMem.logicalOperation (3);
  videoMem.writeByte (3, 0x5C);
  if (buffer[3] != 0xFF13FF13) {
    /* According to existing docs on internet the logical operation shouldn't be
       performed and the expected output would be EC13EC13; however, in an experiment
       on a virtual box, the logical operation was performed!  */
    printf ("Error writing byte with rotation=2, set/reset=0101, host byte=5C, bit mask=00110011, write plane enable=1111, logical op=XOR\n");
    return;
  }

  videoMem.logicalOperation (0);
  for (int i = 0; i < 16; ++i) {
    buffer[i] = 0x01010101;
    uint32_t wpe = createBytePattern (0xFF, i);
    uint32_t expected = 0xEC13EC13 & wpe;
    expected |= buffer[i] & (wpe ^ 0xFFFFFFFF);
    videoMem.writePlaneEnable (i);
    videoMem.writeByte (i, 0x5C);
    if (buffer[i] != expected) {
      printf ("Error writing byte with rotation=2, set/reset=0101, host byte=5C, bit mask=00110011, write plane enable=0x%X\n", i);
      return;
    }
  }
  printf ("Ok\n");
}
