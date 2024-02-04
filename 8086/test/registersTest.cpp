/*
   Author:  Gerard Visser
   e-mail:  visser.gerard(at)gmail.com

   Copyright (C) 2024 Gerard Visser.

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

#include "registersTest.h"
#include <8086/Registers.h>
#include "assertions.h"

void registersTest::getGen (void) {
  printf ("registersTest::getGen: ");

  Registers registers;
  registers.gen[0] = 0x8182;
  registers.gen[1] = 0x8384;
  registers.gen[2] = 0x8586;
  registers.gen[3] = 0x8788;
  registers.gen[4] = 0x090A;
  registers.gen[5] = 0x0B0C;
  registers.gen[6] = 0x0D0E;
  registers.gen[7] = 0x0F10;

  assertTrue (registers.getGen (true, 0) == 0x8182, "Error reading AX.\n");
  assertTrue (registers.getGen (true, 1) == 0x8384, "Error reading CX.\n");
  assertTrue (registers.getGen (true, 2) == 0x8586, "Error reading DX.\n");
  assertTrue (registers.getGen (true, 3) == 0x8788, "Error reading BX.\n");
  assertTrue (registers.getGen (true, 4) == 0x090A, "Error reading SP.\n");
  assertTrue (registers.getGen (true, 5) == 0x0B0C, "Error reading BP.\n");
  assertTrue (registers.getGen (true, 6) == 0x0D0E, "Error reading SI.\n");
  assertTrue (registers.getGen (true, 7) == 0x0F10, "Error reading DI.\n");
  assertTrue (registers.getGen (false, 0) == 0x82, "Error reading AL.\n");
  assertTrue (registers.getGen (false, 1) == 0x84, "Error reading CL.\n");
  assertTrue (registers.getGen (false, 2) == 0x86, "Error reading DL.\n");
  assertTrue (registers.getGen (false, 3) == 0x88, "Error reading BL.\n");
  assertTrue (registers.getGen (false, 4) == 0x81, "Error reading AH.\n");
  assertTrue (registers.getGen (false, 5) == 0x83, "Error reading CH.\n");
  assertTrue (registers.getGen (false, 6) == 0x85, "Error reading DH.\n");
  assertTrue (registers.getGen (false, 7) == 0x87, "Error reading BH.\n");

  printf ("Ok\n");
}

void registersTest::setGen (void) {
  printf ("registersTest::setGen: ");

  Registers registers;
  registers.setGen (true, 0, 0x0102);
  assertTrue (registers.gen[0] == 0x0102, "Error writing AX.\n");
  registers.setGen (true, 1, 0x0304);
  assertTrue (registers.gen[1] == 0x0304, "Error writing CX.\n");
  registers.setGen (true, 2, 0x0506);
  assertTrue (registers.gen[2] == 0x0506, "Error writing DX.\n");
  registers.setGen (true, 3, 0x0708);
  assertTrue (registers.gen[3] == 0x0708, "Error writing BX.\n");
  registers.setGen (true, 4, 0x090A);
  assertTrue (registers.gen[4] == 0x090A, "Error writing SP.\n");
  registers.setGen (true, 5, 0x0B0C);
  assertTrue (registers.gen[5] == 0x0B0C, "Error writing BP.\n");
  registers.setGen (true, 6, 0x0D0E);
  assertTrue (registers.gen[6] == 0x0D0E, "Error writing SI.\n");
  registers.setGen (true, 7, 0x0F10);
  assertTrue (registers.gen[7] == 0x0F10, "Error writing DI.\n");

  registers.setGen (false, 0, 0x8F);
  assertTrue (registers.gen[0] == 0x018F, "Error writing AL.\n");
  registers.setGen (false, 1, 0x8D);
  assertTrue (registers.gen[1] == 0x038D, "Error writing CL.\n");
  registers.setGen (false, 2, 0x8B);
  assertTrue (registers.gen[2] == 0x058B, "Error writing DL.\n");
  registers.setGen (false, 3, 0x89);
  assertTrue (registers.gen[3] == 0x0789, "Error writing BL.\n");
  registers.setGen (false, 4, 0x90);
  assertTrue (registers.gen[0] == 0x908F, "Error writing AH.\n");
  registers.setGen (false, 5, 0x8E);
  assertTrue (registers.gen[1] == 0x8E8D, "Error writing CH.\n");
  registers.setGen (false, 6, 0x8C);
  assertTrue (registers.gen[2] == 0x8C8B, "Error writing DH.\n");
  registers.setGen (false, 7, 0x8A);
  assertTrue (registers.gen[3] == 0x8A89, "Error writing BH.\n");

  printf ("Ok\n");
}
