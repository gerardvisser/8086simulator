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

#include "processorTest.h"
#include "processorTestCode.h"
#include "assertions.h"
#include <8086/Processor.h>
#include "../src/8086/registerAndFlagIds.h"

#define CLEAR_FLAGS(flags) wrapper.clearFlags (flags)
#define FLAGS wrapper.processor.registers ().flags
#define INSTRUCTION_POINTER wrapper.processor.registers ().ip
#define READ_WORD(seg, off) wrapper.readWord (seg, off)
#define REG(x) wrapper.processor.registers ().gen[x]
#define SEG(x) wrapper.processor.registers ().seg[x]
#define SET_FLAGS(flags) wrapper.setFlags (flags)
#define WRITE_WORD(seg, off, val) wrapper.writeWord (seg, off, val)

class ProcessorWrapper {
private:
  VideoMemory m_videoMemory;
  Memory m_memory;
  const Address m_startAddress;

public:
  Processor processor;

  ProcessorWrapper (void) : m_memory (m_videoMemory), processor (m_memory), m_startAddress (0x54, 0) {
    processor.registers ().seg[CS] = m_startAddress.segment;
    processor.registers ().ip = m_startAddress.offset;
    processor.registers ().seg[ES] = 0x1000;
    processor.registers ().seg[SS] = 0x2000;
    processor.registers ().seg[DS] = 0x3000;
    processor.registers ().gen[SP] = 0xFFFE;
  }

  void clearFlags (uint16_t flags) {
    flags ^= 0xFFFF;
    processor.registers ().flags &= flags;
  }

  void loadCode (const uint8_t* bytes, int count) {
    m_memory.writeBytes (m_startAddress, bytes, count);
  }

  int readWord (int segment, int offset) {
    return m_memory.readWord (Address (segment, offset));
  }

  void setFlags (uint16_t flags) {
    processor.registers ().flags |= flags;
  }

  void setGeneralPurposeRegisters (void) {
    processor.registers ().gen[AX] = 0x8001;
    processor.registers ().gen[CX] = 0x1392;
    processor.registers ().gen[DX] = 0xA425;
    processor.registers ().gen[BX] = 0x37B6;
    processor.registers ().gen[BP] = 0x0123;
    processor.registers ().gen[SI] = 0x3456;
    processor.registers ().gen[DI] = 0x6DC9;
  }

  void writeWord (int segment, int offset, int value) {
    m_memory.writeWord (Address (segment, offset), value);
  }
};

void processorTest::interrupt (void) {
  printf ("processorTest::interrupt: ");

  ProcessorWrapper wrapper;
  WRITE_WORD (0, 3 * 4, 0x018B);
  WRITE_WORD (0, 3 * 4 + 2, 0x0070);
  WRITE_WORD (0, 0x19 * 4, 0x0C50);
  WRITE_WORD (0, 0x19 * 4 + 2, 0x020A);

  INSTRUCTION_POINTER = 0x1234;
  WRITE_WORD (SEG (CS), INSTRUCTION_POINTER, 0x19CD);
  SET_FLAGS (F_CARRY | F_AUX_CARRY | F_SIGN | F_INTERRUPT | F_OVERFLOW | F_PARITY | F_ZERO | F_DIRECTION | F_TRAP);

  uint16_t oldFlags = FLAGS;
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 0x0C50, "unexpected IP value after 'int 0x19'\n");
  assertTrue (SEG (CS) == 0x020A, "unexpected CS value after 'int 0x19'\n");
  assertTrue (REG (SP) == 0xFFF8, "unexpected SP value after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1236, "unexpected value at SS:SP after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP) + 2) == 0x0054, "unexpected value at SS:SP+2 after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP) + 4) == oldFlags, "unexpected value at SS:SP+4 after 'int 0x19'\n");
  assertTrue ((oldFlags ^ FLAGS) == (F_INTERRUPT | F_TRAP), "unexpected flags after 'int 0x19'\n");

  INSTRUCTION_POINTER = 0x1234;
  SEG (CS) = 0x0054;
  WRITE_WORD (SEG (CS), INSTRUCTION_POINTER, 0x00CC);
  CLEAR_FLAGS (F_CARRY | F_AUX_CARRY | F_SIGN | F_INTERRUPT | F_OVERFLOW | F_PARITY | F_ZERO | F_DIRECTION | F_TRAP);

  oldFlags = FLAGS;
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 0x018B, "unexpected IP value after 'int 0x19'\n");
  assertTrue (SEG (CS) == 0x0070, "unexpected CS value after 'int 0x19'\n");
  assertTrue (REG (SP) == 0xFFF2, "unexpected SP value after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1235, "unexpected value at SS:SP after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP) + 2) == 0x0054, "unexpected value at SS:SP+2 after 'int 0x19'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP) + 4) == oldFlags, "unexpected value at SS:SP+4 after 'int 0x19'\n");
  assertTrue (FLAGS == oldFlags, "unexpected flags after 'int 0x19'\n");

  printf ("Ok\n");
}

void processorTest::mov (void) {
  printf ("processorTest::mov: ");

  ProcessorWrapper wrapper;
  wrapper.loadCode (processorTestCode::mov, 137);
  wrapper.setGeneralPurposeRegisters ();
  /* AX=0x8001, CX=0x1392, DX=0xA425, BX=0x37B6, BP=0x0123, SI=0x3456, DI=0x6DC9 */

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 2, "unexpected IP value after 'mov dx, bp'\n");
  assertTrue (REG (DX) == 0x123, "unexpected DX value after 'mov dx, bp'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 4, "unexpected IP value after 'mov bx, si'\n");
  assertTrue (REG (BX) == 0x3456, "unexpected BX value after 'mov bx, si'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 6, "unexpected IP value after 'mov bl, cl'\n");
  assertTrue (REG (BX) == 0x3492, "unexpected BX value after 'mov bl, cl'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 8, "unexpected IP value after 'mov cl, ah'\n");
  assertTrue (REG (CX) == 0x1380, "unexpected CX value after 'mov cl, ah'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 10, "unexpected IP value after 'mov sp, dx'\n");
  assertTrue (REG (SP) == 0x123, "unexpected SP value after 'mov sp, dx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 12, "unexpected IP value after 'mov di, cx'\n");
  assertTrue (REG (DI) == 0x1380, "unexpected DI value after 'mov di, cx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 14, "unexpected IP value after 'mov ch, dh'\n");
  assertTrue (REG (CX) == 0x180, "unexpected CX value after 'mov ch, dh'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 16, "unexpected IP value after 'mov bh, al'\n");
  assertTrue (REG (BX) == 0x192, "unexpected BX value after 'mov bh, al'\n");

  wrapper.setGeneralPurposeRegisters ();
  /* AX=0x8001, CX=0x1392, DX=0xA425, BX=0x37B6, BP=0x0123, SI=0x3456, DI=0x6DC9 */

  WRITE_WORD (SEG (DS), REG (BX) + REG (SI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 18, "unexpected IP value after 'mov [bx+si], di'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + REG (SI)) == 0x6DC9, "unexpected memory value after 'mov [bx+si], di'\n");

  WRITE_WORD (SEG (DS), REG (BX) + REG (DI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 20, "unexpected IP value after 'mov [bx+di], dh'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + REG (DI)) == 0xA4, "unexpected memory value after 'mov [bx+di], dh'\n");

  WRITE_WORD (SEG (SS), REG (BP) + REG (SI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 22, "unexpected IP value after 'mov [bp+si], ax'\n");
  assertTrue (READ_WORD (SEG (SS), REG (BP) + REG (SI)) == 0x8001, "unexpected memory value after 'mov [bp+si], ax'\n");

  WRITE_WORD (SEG (SS), REG (BP) + REG (DI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 24, "unexpected IP value after 'mov [bp+di], ah'\n");
  assertTrue (READ_WORD (SEG (SS), REG (BP) + REG (DI)) == 0x80, "unexpected memory value after 'mov [bp+di], ah'\n");

  WRITE_WORD (SEG (DS), REG (SI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 26, "unexpected IP value after 'mov [si], si'\n");
  assertTrue (READ_WORD (SEG (DS), REG (SI)) == 0x3456, "unexpected memory value after 'mov [si], si'\n");

  WRITE_WORD (SEG (DS), REG (DI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 28, "unexpected IP value after 'mov [di], cl'\n");
  assertTrue (READ_WORD (SEG (DS), REG (DI)) == 0x92, "unexpected memory value after 'mov [di], cl'\n");

  WRITE_WORD (SEG (DS), 0x64, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 32, "unexpected IP value after 'mov [0x64], bp'\n");
  assertTrue (READ_WORD (SEG (DS), 0x64) == 0x123, "unexpected memory value after 'mov [0x64], bp'\n");

  WRITE_WORD (SEG (DS), REG (BX), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 34, "unexpected IP value after 'mov [bx], bl'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX)) == 0xB6, "unexpected memory value after 'mov [bx], bl'\n");

  WRITE_WORD (SEG (ES), REG (BX) + REG (SI) + 4, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 38, "unexpected IP value after 'es: mov [bx+si+4], si'\n");
  assertTrue (READ_WORD (SEG (ES), REG (BX) + REG (SI) + 4) == 0x3456, "unexpected memory value after 'es: mov [bx+si+4], si'\n");

  WRITE_WORD (SEG (DS), REG (BX) + REG (DI) + 127, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 41, "unexpected IP value after 'mov [bx+di+127], bl'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + REG (DI) + 127) == 0xB6, "unexpected memory value after 'mov [bx+di+127], bl'\n");

  WRITE_WORD (SEG (DS), REG (BP) + REG (SI) - 2, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 45, "unexpected IP value after 'ds: mov [bp+si-2], bx'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BP) + REG (SI) - 2) == 0x37B6, "unexpected memory value after 'ds: mov [bp+si-2], bx'\n");

  WRITE_WORD (SEG (CS), REG (BP) + REG (SI) - 128, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 49, "unexpected IP value after 'cs: mov [bp+si-128], bh'\n");
  assertTrue (READ_WORD (SEG (CS), REG (BP) + REG (SI) - 128) == 0x37, "unexpected memory value after 'cs: mov [bp+si-128], bh'\n");

  REG (SP) = 0xFFFE;
  WRITE_WORD (SEG (SS), REG (BP), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 52, "unexpected IP value after 'mov [bp+0], sp'\n");
  assertTrue (READ_WORD (SEG (SS), REG (BP)) == 0xFFFE, "unexpected memory value after 'mov [bp+0], sp'\n");

  WRITE_WORD (SEG (SS), REG (SI) + 0x100, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 57, "unexpected IP value after 'ss: mov [si+0x100], ax'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SI) + 0x100) == 0x8001, "unexpected memory value after 'ss: mov [si+0x100], ax'\n");

  WRITE_WORD (SEG (DS), REG (BX) - 0x100, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 61, "unexpected IP value after 'mov [bx-0x100], ah'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) - 0x100) == 0x80, "unexpected memory value after 'mov [bx-0x100], ah'\n");

  WRITE_WORD (SEG (DS), REG (DI), 0xF5D8);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 63, "unexpected IP value after 'mov di, [di]'\n");
  assertTrue (REG (DI) == 0xF5D8, "unexpected DI value after 'mov di, [di]'\n");

  WRITE_WORD (SEG (DS), REG (SI) - 2, 0x7EC3);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 66, "unexpected IP value after 'mov ch, [si-2]'\n");
  assertTrue (REG (CX) == 0xC392, "unexpected CX value after 'mov ch, [si-2]'\n");

  WRITE_WORD (SEG (DS), REG (BX) + 0x200, 0x6D7A);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 70, "unexpected IP value after 'mov si, [bx+0x200]'\n");
  assertTrue (REG (SI) == 0x6D7A, "unexpected SI value after 'mov si, [bx+0x200]'\n");

  WRITE_WORD (SEG (DS), 0x100, 0x304E);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 73, "unexpected IP value after 'mov ax, [0x100]'\n");
  assertTrue (REG (AX) == 0x304E, "unexpected AX value after 'mov ax, [0x100]'\n");

  WRITE_WORD (SEG (DS), 0x100, 0xBBE0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 76, "unexpected IP value after 'mov al, [0x100]'\n");
  assertTrue (REG (AX) == 0x30E0, "unexpected AX value after 'mov al, [0x100]'\n");

  WRITE_WORD (SEG (DS), 0x102, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 79, "unexpected IP value after 'mov [0x102], ax'\n");
  assertTrue (READ_WORD (SEG (DS), 0x102) == 0x30E0, "unexpected memory value after 'mov [0x102], ax'\n");

  WRITE_WORD (SEG (DS), 0x104, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 82, "unexpected IP value after 'mov [0x104], al'\n");
  assertTrue (READ_WORD (SEG (DS), 0x104) == 0xE0, "unexpected memory value after 'mov [0x104], al'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 85, "unexpected IP value after 'mov bp, 0xBB39'\n");
  assertTrue (REG (BP) == 0xBB39, "unexpected BP value after 'mov bp, 0xBB39'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 88, "unexpected IP value after 'mov bx, 0x90F4'\n");
  assertTrue (REG (BX) == 0x90F4, "unexpected BX value after 'mov bx, 0x90F4'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 90, "unexpected IP value after 'mov dh, 0x56'\n");
  assertTrue (REG (DX) == 0x5625, "unexpected DX value after 'mov dh, 0x56'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 92, "unexpected IP value after 'mov ah, 0xBE'\n");
  assertTrue (REG (AX) == 0xBEE0, "unexpected AX value after 'mov ah, 0xBE'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 96, "unexpected IP value after 'mov ax, 0x2DEF'\n");
  assertTrue (REG (AX) == 0x2DEF, "unexpected AX value after 'mov ax, 0x2DEF'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 99, "unexpected IP value after 'mov dl, 0x26'\n");
  assertTrue (REG (DX) == 0x5626, "unexpected DX value after 'mov dl, 0x26'\n");

  wrapper.setGeneralPurposeRegisters ();
  /* AX=0x8001, CX=0x1392, DX=0xA425, BX=0x37B6, BP=0x0123, SI=0x3456, DI=0x6DC9 */

  WRITE_WORD (SEG (DS), REG (DI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 103, "unexpected IP value after 'mov word ptr [di], 0x1069'\n");
  assertTrue (READ_WORD (SEG (DS), REG (DI)) == 0x1069, "unexpected memory value after 'mov word ptr [di], 0x1069'\n");

  WRITE_WORD (SEG (SS), REG (BP) + REG (DI), 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 106, "unexpected IP value after 'mov byte ptr [bp+di], 0x21'\n");
  assertTrue (READ_WORD (SEG (SS), REG (BP) + REG (DI)) == 0x21, "unexpected memory value after 'mov byte ptr [bp+di], 0x21'\n");

  WRITE_WORD (SEG (DS), REG (BX) + REG (DI) + 2, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 111, "unexpected IP value after 'mov word ptr [bx+di+2], 0xC259'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + REG (DI) + 2) == 0xC259, "unexpected memory value after 'mov word ptr [bx+di+2], 0xC259'\n");

  WRITE_WORD (SEG (SS), REG (BP) + 4, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 115, "unexpected IP value after 'mov byte ptr [bp+4], 0xB7'\n");
  assertTrue (READ_WORD (SEG (SS), REG (BP) + 4) == 0xB7, "unexpected memory value after 'mov byte ptr [bp+4], 0xB7'\n");

  WRITE_WORD (SEG (DS), REG (SI) + 0x120, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 121, "unexpected IP value after 'mov word ptr [si+0x120], 0xC8A2'\n");
  assertTrue (READ_WORD (SEG (DS), REG (SI) + 0x120) == 0xC8A2, "unexpected memory value after 'mov word ptr [si+0x120], 0xC8A2'\n");

  WRITE_WORD (SEG (DS), REG (BX) + REG (SI) + 0x90, 0);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 126, "unexpected IP value after 'mov byte ptr [bx+si+0x90], 0xBD'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + REG (SI) + 0x90) == 0xBD, "unexpected memory value after 'mov byte ptr [bx+si+0x90], 0xBD'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 128, "unexpected IP value after 'mov ss, dx'\n");
  assertTrue (SEG (SS) == 0xA425, "unexpected SS value after 'mov ss, dx'\n");

  WRITE_WORD (SEG (DS), REG (BX) + 2, 0xC986);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 131, "unexpected IP value after 'mov ds, [bx+2]'\n");
  assertTrue (SEG (DS) == 0xC986, "unexpected DS value after 'mov ds, [bx+2]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 133, "unexpected IP value after 'mov bp, cs'\n");
  assertTrue (REG (BP) == 0x54, "unexpected BP value after 'mov bp, cs'\n");

  WRITE_WORD (SEG (DS), REG (SI) + 0x100, 0xFFFF);
  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 137, "unexpected IP value after 'mov [si+0x100], es'\n");
  assertTrue (READ_WORD (SEG (DS), REG (SI) + 0x100) == 0x1000, "unexpected memory value after 'mov [si+0x100], es'\n");

  printf ("Ok\n");
}

void processorTest::pushfPopf (void) {
  printf ("processorTest::pushfPopf: ");

  ProcessorWrapper wrapper;
  wrapper.loadCode (processorTestCode::pushfPopf, 2);
  SET_FLAGS (F_CARRY | F_AUX_CARRY | F_SIGN | F_INTERRUPT | F_OVERFLOW);
  CLEAR_FLAGS (F_PARITY | F_ZERO | F_TRAP | F_DIRECTION);

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 1, "unexpected IP value after 'pushf'\n");
  assertTrue (REG (SP) == 0xFFFC, "unexpected SP value after 'pushf'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0xFA93, "unexpected top of the stack value after 'pushf'\n");

  WRITE_WORD (SEG (SS), REG (SP), 0x046C);

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 2, "unexpected IP value after 'popf'\n");
  assertTrue (REG (SP) == 0xFFFE, "unexpected SP value after 'popf'\n");
  assertTrue (FLAGS == 0xF446, "unexpected flags value after 'popf'\n");

  printf ("Ok\n");
}

void processorTest::pushPop (void) {
  printf ("processorTest::pushPop: ");

  ProcessorWrapper wrapper;
  wrapper.loadCode (processorTestCode::pushPop, 55);
  wrapper.setGeneralPurposeRegisters ();
  /* AX=0x8001, CX=0x1392, DX=0xA425, BX=0x37B6, BP=0x0123, SI=0x3456, DI=0x6DC9 */

  WRITE_WORD (SEG (DS), REG (BX), 0x29CF);
  WRITE_WORD (SEG (DS), REG (BX) + 2, 0x1B33);
  WRITE_WORD (SEG (DS), REG (BX) + 0x80, 0x57DD);
  WRITE_WORD (SEG (SS), 0x3456, 0xA066);

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 1, "unexpected IP value after 'push ax'\n");
  assertTrue (REG (SP) == 0xFFFC, "unexpected SP value after 'push ax'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x8001, "unexpected top of the stack value after 'push ax'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 2, "unexpected IP value after 'push cx'\n");
  assertTrue (REG (SP) == 0xFFFA, "unexpected SP value after 'push cx'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1392, "unexpected top of the stack value after 'push cx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 3, "unexpected IP value after 'push dx'\n");
  assertTrue (REG (SP) == 0xFFF8, "unexpected SP value after 'push dx'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0xA425, "unexpected top of the stack value after 'push dx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 4, "unexpected IP value after 'push bx'\n");
  assertTrue (REG (SP) == 0xFFF6, "unexpected SP value after 'push bx'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x37B6, "unexpected top of the stack value after 'push bx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 5, "unexpected IP value after 'push sp'\n");
  assertTrue (REG (SP) == 0xFFF4, "unexpected SP value after 'push sp'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0xFFF6, "unexpected top of the stack value after 'push sp'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 6, "unexpected IP value after 'push bp'\n");
  assertTrue (REG (SP) == 0xFFF2, "unexpected SP value after 'push bp'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x0123, "unexpected top of the stack value after 'push bp'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 7, "unexpected IP value after 'push si'\n");
  assertTrue (REG (SP) == 0xFFF0, "unexpected SP value after 'push si'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x3456, "unexpected top of the stack value after 'push si'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 8, "unexpected IP value after 'push di'\n");
  assertTrue (REG (SP) == 0xFFEE, "unexpected SP value after 'push di'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x6DC9, "unexpected top of the stack value after 'push di'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 9, "unexpected IP value after 'push es'\n");
  assertTrue (REG (SP) == 0xFFEC, "unexpected SP value after 'push es'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1000, "unexpected top of the stack value after 'push es'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 10, "unexpected IP value after 'push cs'\n");
  assertTrue (REG (SP) == 0xFFEA, "unexpected SP value after 'push cs'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x0054, "unexpected top of the stack value after 'push cs'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 11, "unexpected IP value after 'push ss'\n");
  assertTrue (REG (SP) == 0xFFE8, "unexpected SP value after 'push ss'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x2000, "unexpected top of the stack value after 'push ss'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 12, "unexpected IP value after 'push ds'\n");
  assertTrue (REG (SP) == 0xFFE6, "unexpected SP value after 'push ds'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x3000, "unexpected top of the stack value after 'push ds'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 14, "unexpected IP value after 'push [bx]'\n");
  assertTrue (REG (SP) == 0xFFE4, "unexpected SP value after 'push [bx]'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x29CF, "unexpected top of the stack value after 'push [bx]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 17, "unexpected IP value after 'push [bx+2]'\n");
  assertTrue (REG (SP) == 0xFFE2, "unexpected SP value after 'push [bx+2]'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1B33, "unexpected top of the stack value after 'push [bx+2]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 21, "unexpected IP value after 'push [bx+0x80]'\n");
  assertTrue (REG (SP) == 0xFFE0, "unexpected SP value after 'push [bx+0x80]'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x57DD, "unexpected top of the stack value after 'push [bx+0x80]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 23, "unexpected IP value after 'db 0xFF, 0xF1'\n");
  assertTrue (REG (SP) == 0xFFDE, "unexpected SP value after 'db 0xFF, 0xF1'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x1392, "unexpected top of the stack value after 'db 0xFF, 0xF1'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 25, "unexpected IP value after 'push 0x7F'\n");
  assertTrue (REG (SP) == 0xFFDC, "unexpected SP value after 'push 0x7F'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x007F, "unexpected top of the stack value after 'push 0x7F'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 27, "unexpected IP value after 'push -0x80'\n");
  assertTrue (REG (SP) == 0xFFDA, "unexpected SP value after 'push -0x80'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0xFF80, "unexpected top of the stack value after 'push -0x80'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 30, "unexpected IP value after 'push 0x80'\n");
  assertTrue (REG (SP) == 0xFFD8, "unexpected SP value after 'push 0x80'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x0080, "unexpected top of the stack value after 'push 0x80'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 33, "unexpected IP value after 'push 0x955A'\n");
  assertTrue (REG (SP) == 0xFFD6, "unexpected SP value after 'push 0x955A'\n");
  assertTrue (READ_WORD (SEG (SS), REG (SP)) == 0x955A, "unexpected top of the stack value after 'push 0x955A'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 35, "unexpected IP value after 'db 0x8F, 0xC2'\n");
  assertTrue (REG (SP) == 0xFFD8, "unexpected SP value after 'db 0x8F, 0xC2'\n");
  assertTrue (REG (DX) == 0x955A, "unexpected DX value after 'db 0x8F, 0xC2'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 37, "unexpected IP value after 'pop [bx]'\n");
  assertTrue (REG (SP) == 0xFFDA, "unexpected SP value after 'pop [bx]'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX)) == 0x0080, "unexpected memory value after 'pop [bx]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 40, "unexpected IP value after 'pop [bx+2]'\n");
  assertTrue (REG (SP) == 0xFFDC, "unexpected SP value after 'pop [bx+2]'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + 2) == 0xFF80, "unexpected memory value after 'pop [bx+2]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 44, "unexpected IP value after 'pop [bx+0x80]'\n");
  assertTrue (REG (SP) == 0xFFDE, "unexpected SP value after 'pop [bx+0x80]'\n");
  assertTrue (READ_WORD (SEG (DS), REG (BX) + 0x80) == 0x007F, "unexpected memory value after 'pop [bx+0x80]'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 45, "unexpected IP value after 'pop ds'\n");
  assertTrue (REG (SP) == 0xFFE0, "unexpected SP value after 'pop ds'\n");
  assertTrue (SEG (DS) == 0x1392, "unexpected DS value after 'pop ds'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 46, "unexpected IP value after 'pop es'\n");
  assertTrue (REG (SP) == 0xFFE2, "unexpected SP value after 'pop es'\n");
  assertTrue (SEG (ES) == 0x57DD, "unexpected ES value after 'pop es'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 47, "unexpected IP value after 'pop di'\n");
  assertTrue (REG (SP) == 0xFFE4, "unexpected SP value after 'pop di'\n");
  assertTrue (REG (DI) == 0x1B33, "unexpected DI value after 'pop di'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 48, "unexpected IP value after 'pop si'\n");
  assertTrue (REG (SP) == 0xFFE6, "unexpected SP value after 'pop si'\n");
  assertTrue (REG (SI) == 0x29CF, "unexpected SI value after 'pop si'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 49, "unexpected IP value after 'pop bp'\n");
  assertTrue (REG (SP) == 0xFFE8, "unexpected SP value after 'pop bp'\n");
  assertTrue (REG (BP) == 0x3000, "unexpected BP value after 'pop bp'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 50, "unexpected IP value after 'pop bx'\n");
  assertTrue (REG (SP) == 0xFFEA, "unexpected SP value after 'pop bx'\n");
  assertTrue (REG (BX) == 0x2000, "unexpected BX value after 'pop bx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 51, "unexpected IP value after 'pop dx'\n");
  assertTrue (REG (SP) == 0xFFEC, "unexpected SP value after 'pop dx'\n");
  assertTrue (REG (DX) == 0x0054, "unexpected DX value after 'pop dx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 52, "unexpected IP value after 'pop cx'\n");
  assertTrue (REG (SP) == 0xFFEE, "unexpected SP value after 'pop cx'\n");
  assertTrue (REG (CX) == 0x1000, "unexpected CX value after 'pop cx'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 53, "unexpected IP value after 'pop ax'\n");
  assertTrue (REG (SP) == 0xFFF0, "unexpected SP value after 'pop ax'\n");
  assertTrue (REG (AX) == 0x6DC9, "unexpected AX value after 'pop ax'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 54, "unexpected IP value after 'pop sp'\n");
  assertTrue (REG (SP) == 0x3456, "unexpected SP value after 'pop sp'\n");

  wrapper.processor.executeNextInstruction ();
  assertTrue (INSTRUCTION_POINTER == 55, "unexpected IP value after 'pop ss'\n");
  assertTrue (REG (SP) == 0x3458, "unexpected SP value after 'pop ss'\n");
  assertTrue (SEG (SS) == 0xA066, "unexpected SS value after 'pop ss'\n");

  printf ("Ok\n");
}
