#include "videoModes.h"

static void setCGADac (VideoCard& videoCard, Memory& memory) {
  Address srcAddress (0xC000, 0);
  int dacOffset = memory.readWord (srcAddress);
  srcAddress.offset = dacOffset;

  videoCard.writeByte (0x3C8, 0x00);
  for (int i = 0; i < 192; ++i) {
    int b = memory.readByte (srcAddress);
    videoCard.writeByte (0x3C9, b);
    ++srcAddress;
  }
  for (int i = 0; i < 576; ++i) {
    videoCard.writeByte (0x3C9, 0);
  }
}

static void setCGAPalette_2colours (VideoCard& videoCard) {
  videoCard.readByte (0x3DA);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x00);
  for (int i = 1; i < 16; ++i) {
    videoCard.writeByte (0x3C0, i);
    videoCard.writeByte (0x3C0, 0x17);
  }
  //Gebeurt later als de registers 0x10 en 0x14 worden gezet:
  //videoCard.writeByte (0x3C0, 0x20);
  //videoCard.readByte (0x3DA);
}

static void setCGAPalette_4colours (VideoCard& videoCard) {
  videoCard.readByte (0x3DA);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x00);
  videoCard.writeByte (0x3C0, 1);
  videoCard.writeByte (0x3C0, 0x13);
  videoCard.writeByte (0x3C0, 2);
  videoCard.writeByte (0x3C0, 0x15);
  videoCard.writeByte (0x3C0, 3);
  videoCard.writeByte (0x3C0, 0x17);
  videoCard.writeByte (0x3C0, 4);
  videoCard.writeByte (0x3C0, 0x02);
  videoCard.writeByte (0x3C0, 5);
  videoCard.writeByte (0x3C0, 0x04);
  videoCard.writeByte (0x3C0, 6);
  videoCard.writeByte (0x3C0, 0x06);
  videoCard.writeByte (0x3C0, 7);
  videoCard.writeByte (0x3C0, 0x07);
  for (int i = 8; i < 16; ++i) {
    videoCard.writeByte (0x3C0, i);
    videoCard.writeByte (0x3C0, i + 8);
  }
  //Gebeurt later als de registers 0x10 en 0x14 worden gezet:
  //videoCard.writeByte (0x3C0, 0x20);
  //videoCard.readByte (0x3DA);
}

static void setEGADac (VideoCard& videoCard, Memory& memory) {
  Address srcAddress (0xC000, 0x2);
  int dacOffset = memory.readWord (srcAddress);
  srcAddress.offset = dacOffset;

  videoCard.writeByte (0x3C8, 0x00);
  for (int i = 0; i < 192; ++i) {
    int b = memory.readByte (srcAddress);
    videoCard.writeByte (0x3C9, b);
    ++srcAddress;
  }
  for (int i = 0; i < 576; ++i) {
    videoCard.writeByte (0x3C9, 0);
  }
}

static void setEGAPalette (VideoCard& videoCard) {
  videoCard.readByte (0x3DA);
  for (int i = 0; i < 6; ++i) {
    videoCard.writeByte (0x3C0, i);
    videoCard.writeByte (0x3C0, i);
  }
  videoCard.writeByte (0x3C0, 6);
  videoCard.writeByte (0x3C0, 0x14);
  videoCard.writeByte (0x3C0, 7);
  videoCard.writeByte (0x3C0, 7);
  for (int i = 8; i < 16; ++i) {
    videoCard.writeByte (0x3C0, i);
    videoCard.writeByte (0x3C0, i + 0x30);
  }
  //Gebeurt later als de registers 0x10 en 0x14 worden gezet:
  //videoCard.writeByte (0x3C0, 0x20);
  //videoCard.readByte (0x3DA);
}

static void setTextModeDac (VideoCard& videoCard, Memory& memory) {
  Address srcAddress (0xC000, 0x2);
  int dacOffset = memory.readWord (srcAddress);
  srcAddress.offset = dacOffset;

  videoCard.writeByte (0x3C8, 0x00);
  for (int i = 0; i < 192; ++i) {
    int b = memory.readByte (srcAddress);
    videoCard.writeByte (0x3C9, b);
    ++srcAddress;
  }

  srcAddress.offset = 4;
  dacOffset = memory.readWord (srcAddress);
  srcAddress.offset = dacOffset + 192;
  for (int i = 0; i < 576; ++i) {
    int b = memory.readByte (srcAddress);
    videoCard.writeByte (0x3C9, b);
    ++srcAddress;
  }
}

static void setVGADac (VideoCard& videoCard, Memory& memory) {
  Address srcAddress (0xC000, 4);
  int dacOffset = memory.readWord (srcAddress);
  srcAddress.offset = dacOffset;

  videoCard.writeByte (0x3C8, 0x00);
  for (int i = 0; i < 768; ++i) {
    int b = memory.readByte (srcAddress);
    videoCard.writeByte (0x3C9, b);
    ++srcAddress;
  }
}

static void setVGAPalette (VideoCard& videoCard) {
  videoCard.readByte (0x3DA);
  for (int i = 0; i < 16; ++i) {
    videoCard.writeByte (0x3C0, i);
    videoCard.writeByte (0x3C0, i);
  }
  //Gebeurt later als de registers 0x10 en 0x14 worden gezet:
  //videoCard.writeByte (0x3C0, 0x20);
  //videoCard.readByte (0x3DA);
}

/*
Memory map (3CE, 6, bits: 3, 2): 3 (B800, 32kiB)
Grafisch (3CE, 6, bit: 0): FALSE
Shift mode (3CE, 5, bits: 6, 5): SINGLE_SHIFT (niet belangrijk als in tekstmodus)

Memory mode (3C4, 4, bits: 3, 2): ODD_EVEN
Write Plane Enable (3C4, 2): 0011b
Narrow Characters (3C4, 1, bit: 0): FALSE [*]

Scan Doubling (3D4, 9, bit: 7): FALSE
Horizontal Total (3D4, 0): 4D
Vertical Total (3D4, 6): 1DF
Horizontal End (3D4, 1): 4F
Vertical End (3D4, 12): 18F
Overflow (3D4, 7): 1F
Cga Addressing (3D4, 17, bit: 0, inverted): FALSE

Wide Pixels (3C0, 10, bit: 6): FALSE
 */
static void setMode03 (VideoCard& videoCard, Memory& memory) {
  Address srcAddress (0xC000, 0xA);
  int offset8x16Font = memory.readWord (srcAddress);
  srcAddress.offset = offset8x16Font;

  Address dstAddress (0xA000, 0);

  videoCard.writeWord (0x3C4, 0x0402); /* enable plane 2 */
  for (int i = 0; i < 0x100; ++i) {
    for (int j = 0; j < 0x10; ++j) {
      int b = memory.readByte (srcAddress);
      ++srcAddress;
      memory.writeByte (dstAddress, b);
      ++dstAddress;
    }
    dstAddress += 0x10;
  }

  dstAddress.offset = 0;
  videoCard.writeWord (0x3C4, 0x0302); /* enable planes 1, 0 */
  videoCard.writeWord (0x3C4, 0x0204); /* odd/even */
  for (int i = 0; i < 0x4000; ++i) {
    memory.writeWord (dstAddress, 0x0720);
    dstAddress += 2;
  }
  setTextModeDac (videoCard, memory);
  setEGAPalette (videoCard);

  videoCard.readByte (0x3DA);
  videoCard.writeByte (0x3C0, 0x30);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x34);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x20);
  videoCard.readByte (0x3DA);

  videoCard.writeWord (0x3D4, 0x5500);//9pix char mode: 4D00
  videoCard.writeWord (0x3D4, 0x4F01);
  videoCard.writeWord (0x3D4, 0xDF06);
  videoCard.writeWord (0x3D4, 0x1F07);
  videoCard.writeWord (0x3D4, 0x4F09);
  videoCard.writeWord (0x3D4, 0x8F12);
  videoCard.writeWord (0x3D4, 0xA317);
  videoCard.writeWord (0x3CE, 0x1005); /* not necessary */
  videoCard.writeWord (0x3CE, 0x0E06); /* Memory map: B800, Text */
  /* afgerond?  */
  /*
  videoCard.writeByte (0x3C4, 1);
  int b = videoCard.readByte (0x3C5);
  b &= 0xFE;
  videoCard.writeByte (0x3C5, b); // 9 pixels wide characters
  */

  /* int 43h zetten; bios variabelen zetten  */
}

/*
Memory map (3CE, 6, bits: 3, 2): 3 (B800, 32kiB)
Grafisch (3CE, 6, bit: 0): TRUE
Shift mode (3CE, 5, bits: 6, 5): INTERLEAVED_SHIFT

Memory mode (3C4, 4, bits: 3, 2): ODD_EVEN
Write Plane Enable (3C4, 2): 0011b
Narrow Characters (3C4, 1, bit: 0): TRUE

Scan Doubling (3D4, 9, bit: 7): TRUE
Horizontal Total (3D4, 0): 28
Vertical Total (3D4, 6): 1DF
Horizontal End (3D4, 1): 27
Vertical End (3D4, 12): 18F
Overflow (3D4, 7): 1F
Cga Addressing (3D4, 17, bit: 0, inverted): TRUE

Wide Pixels (3C0, 10, bit: 6): FALSE
 */
static void setMode04 (VideoCard& videoCard, Memory& memory) {
  Address dstAddress (0xA000, 0);
  videoCard.writeWord (0x3C4, 0x0F02); /* enable planes 3, 2, 1, 0 */
  for (int i = 0; i < 0x8000; ++i) {
    memory.writeWord (dstAddress, 0);
    dstAddress += 2;
  }
  videoCard.writeWord (0x3C4, 0x0302); /* enable planes 1, 0 */
  videoCard.writeWord (0x3C4, 0x0204); /* odd/even */
  setCGADac (videoCard, memory);
  setCGAPalette_4colours (videoCard);

  videoCard.readByte (0x3DA);
  videoCard.writeByte (0x3C0, 0x30);
  videoCard.writeByte (0x3C0, 1);
  videoCard.writeByte (0x3C0, 0x34);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x20);
  videoCard.readByte (0x3DA);

  videoCard.writeWord (0x3D4, 0x2800);
  videoCard.writeWord (0x3D4, 0x2701);
  videoCard.writeWord (0x3D4, 0xDF06);
  videoCard.writeWord (0x3D4, 0x1F07);
  videoCard.writeWord (0x3D4, 0xC009); /* Different from VGA setting (C1) */
  videoCard.writeWord (0x3D4, 0x8F12);
  videoCard.writeWord (0x3D4, 0xA217);
  videoCard.writeWord (0x3CE, 0x3005); /* INTERLEAVED_SHIFT */
  videoCard.writeWord (0x3CE, 0x0F06); /* Memory map: B800, Graphical */
}

/*
Memory map (3CE, 6, bits: 3, 2): 1 (A000, 64kiB)
Grafisch (3CE, 6, bit: 0): TRUE
Shift mode (3CE, 5, bits: 6, 5): SINGLE_SHIFT (0)

Memory mode (3C4, 4, bits: 3, 2): PLANAR
Write Plane Enable (3C4, 2): 1111b
Narrow Characters (3C4, 1, bit: 0): TRUE

Scan Doubling (3D4, 9, bit: 7): FALSE
Horizontal Total (3D4, 0): 55
Vertical Total (3D4, 6): 1DF
Horizontal End (3D4, 1): 4F
Vertical End (3D4, 12): 1DF
Overflow (3D4, 7): 1F (different from VGA setting)
Cga Addressing (3D4, 17, bit: 0, inverted): FALSE

Wide Pixels (3C0, 10, bit: 6): FALSE
 */
static void setMode12 (VideoCard& videoCard, Memory& memory) {
  Address dstAddress (0xA000, 0);
  videoCard.writeWord (0x3C4, 0x0F02); /* enable planes 3, 2, 1, 0 */
  for (int i = 0; i < 0x8000; ++i) {
    memory.writeWord (dstAddress, 0);
    dstAddress += 2;
  }
  setEGADac (videoCard, memory);
  setEGAPalette (videoCard);

  videoCard.readByte (0x3DA);
  videoCard.writeByte (0x3C0, 0x30);
  videoCard.writeByte (0x3C0, 1);
  videoCard.writeByte (0x3C0, 0x34);
  videoCard.writeByte (0x3C0, 0);
  videoCard.writeByte (0x3C0, 0x20);
  videoCard.readByte (0x3DA);

  videoCard.writeWord (0x3D4, 0x5500);
  videoCard.writeWord (0x3D4, 0x4F01);
  videoCard.writeWord (0x3D4, 0xDF06);
  videoCard.writeWord (0x3D4, 0x1F07);
  videoCard.writeWord (0x3D4, 0x4009);
  videoCard.writeWord (0x3D4, 0xDF12);
  videoCard.writeWord (0x3D4, 0xE317);
  //videoCard.writeWord (0x3CE, 0x0005); /* SINGLE_SHIFT, already set  */
  videoCard.writeWord (0x3CE, 0x0506); /* Memory map: A000, Graphical */
}

void videoModes::setMode (VideoCard& videoCard, Memory& memory, int mode) {
  videoCard.writeWord (0x3C4, 0x2101); /* Disable video */
  for (int i = 0; i < 6; ++i) {
    videoCard.writeWord (0x3CE, i);
  }
  videoCard.writeWord (0x3CE, 0x0406); /* Memory map: A000 */
  videoCard.writeWord (0x3CE, 0x0F07);
  videoCard.writeWord (0x3CE, 0xFF08);
  videoCard.writeWord (0x3C4, 0x0604); /* planar */

  switch (mode) {
  case 3:
    setMode03 (videoCard, memory);
    break;

  case 4:
    setMode04 (videoCard, memory);
    break;

  case 0x12:
    setMode12 (videoCard, memory);
    break;
  }

  videoCard.writeByte (0x3C4, 1);
  int b = videoCard.readByte (0x3C5);
  b &= 0xDF;
  videoCard.writeByte (0x3C5, b); /* Enable video */
}
