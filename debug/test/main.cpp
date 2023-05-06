#include <cstdio>
#include <iostream>
#include <string>
#include <8086/Machine.h>
#include "videoModes.h"

static void printString (Memory& memory, const string& str);
static void writeSomePixels04 (Memory& memory);
static void writeSomePixels12 (Memory& memory, VideoCard& videoCard);

int main (int argc, char** args, char** env) {
  Machine machine ("Dit is een testscherm");
  VideoCard& videoCard = machine.videoCard ();
  Memory& memory = machine.memory ();

  videoModes::setMode (videoCard, memory, 0x3);
  //videoCard.writeWord (0x3D4, 0x4009);
  //writeSomePixels04 (memory/*, videoCard*/);

  string input;
  printf ("> ");
  getline (std::cin, input);
  while (input != "quit") {
    printString (memory, input);

    printf ("> ");
    getline (std::cin, input);
  }
  printf ("Quitting\n");

  return 0;
}

static void printString (Memory& memory, const string& str) {
  Address address (0xB800, 0);
  for (int i = 0; i < str.length (); ++i) {
    memory.writeByte (address, str[i]);
    address += 2;
  }
}

static void writeSomePixels04 (Memory& memory) {
  int pix[] = {0x55, 0xAA, 0xFF, 0xCC};
  int offs[] = {0, 0x2000, 0x50, 0x2050};
  Address address (0xB800, 0);

  for (int i = 0; i < 4; ++i) {
    address.offset = offs[i];
    for (int j = 0; j < 16; ++j) {
      memory.writeByte (address, pix[i]);
      ++address;
    }
  }
}

static void writeSomePixels12 (Memory& memory, VideoCard& videoCard) {
  Address address (0xA000, 0);

  videoCard.writeWord (0x3CE, 0x0205); /* write mode 2 */
  for (int i = 0; i < 15; ++i) {
    for (int j = 0; j < 16; ++j) {
      memory.writeByte (address, i + 1);
      ++address;
    }
    address += 0x42;
  }

  address += 850;
  for (int k = 0; k < 24; ++k) {
    for (int j = 0; j < 15; ++j) {
      for (int i = 0; i < 3; ++i) {
        memory.writeByte (address, j + 1);
        ++address;
      }
      ++address;
    }
    address += 20;
  }
}
