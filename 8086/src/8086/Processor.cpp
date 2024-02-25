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

#include <8086/Processor.h>
#include "registerAndFlagIds.h"

#define instructionHasMemoryOperand(modrmByte) (modrmByte >> 6 != 3)

Processor::Processor (Memory& memory) : m_memory (memory), m_segmentOverride (-1) {
}

Processor::~Processor (void) {
}

Address Processor::calculateOperandAddress (void) {
  Address address;
  int modrmByte = m_instruction[1];
  switch (modrmByte & 7) {
  case 0:
    address.segment = m_registers.seg[DS];
    address.offset = m_registers.gen[BX] + m_registers.gen[SI];
    break;

  case 1:
    address.segment = m_registers.seg[DS];
    address.offset = m_registers.gen[BX] + m_registers.gen[DI];
    break;

  case 2:
    address.segment = m_registers.seg[SS];
    address.offset = m_registers.gen[BP] + m_registers.gen[SI];
    break;

  case 3:
    address.segment = m_registers.seg[SS];
    address.offset = m_registers.gen[BP] + m_registers.gen[DI];
    break;

  case 4:
    address.segment = m_registers.seg[DS];
    address.offset = m_registers.gen[SI];
    break;

  case 5:
    address.segment = m_registers.seg[DS];
    address.offset = m_registers.gen[DI];
    break;

  case 6:
    address.segment = m_registers.seg[SS];
    address.offset = m_registers.gen[BP];
    break;

  case 7:
    address.segment = m_registers.seg[DS];
    address.offset = m_registers.gen[BX];
  }

  switch (modrmByte >> 6) {
  case 0:
    if ((modrmByte & 7) == 6) {
      address.segment = m_registers.seg[DS];
      address.offset = m_instruction.getWord (2);
      m_displacementBytes = 2;
      m_registers.ip += 2;
    } else {
      m_displacementBytes = 0;
    }
    break;

  case 1:
    address.offset += m_instruction.getSignedByteAsWord (2);
    m_displacementBytes = 1;
    ++m_registers.ip;
    break;

  case 2:
    address.offset += m_instruction.getWord (2);
    m_displacementBytes = 2;
    m_registers.ip += 2;
  }

  if (m_segmentOverride > -1) {
    address.segment = m_registers.seg[m_segmentOverride];
  }
  return address;
}

void Processor::execute00xx (void) {
  switch (m_instruction[0] & 6) {
  case 6: /* 00xx x11x */
    /* TODO */
    break;

  case 4: /* 00xx x10x */
    /* TODO */
    break;

  default: /* 00xx x0xx */
    /* TODO */
    ;
  }
}

void Processor::execute1000 (void) {
  switch (m_instruction[0] & 0xF) {
  case 0:
  case 1:
  case 2:
  case 3:
    /* TODO */
    break;

  case 4:
  case 5:
    /* TODO */
    break;

  case 6:
  case 7:
    /* TODO */
    break;

  case 8:
  case 9:
    executeMovRmReg ();
    break;

  case 0xA:
  case 0xB:
    executeMovRegRm ();
    break;

  case 0xC:
    executeMovRmSeg ();
    break;

  case 0xD:
    /* TODO */
    break;

  case 0xE:
    executeMovSegRm ();
    break;

  default:
    /* TODO */
    ;
  }
}

void Processor::execute1001 (void) {
  switch (m_instruction[0] & 0xF) {
  case 0:
    ++m_registers.ip;
    break;

  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    /* TODO */
    break;

  case 8:
    /* TODO */
    break;

  case 9:
    /* TODO */
    break;

  case 0xA:
    /* TODO */
    break;

  case 0xB:
    /* TODO */
    break;

  case 0xC:
    /* TODO */
    break;

  case 0xD:
    /* TODO */
    break;

  case 0xE:
    /* TODO */
    break;

  default:
    /* TODO */
    break;
  }
}

void Processor::execute1010 (void) {
  switch (m_instruction[0] & 0xF) {
  case 0:
  case 1:
    executeMovAccMem ();
    break;

  case 2:
  case 3:
    executeMovMemAcc ();
    break;

  case 4:
    /* TODO */
    break;

  case 5:
    /* TODO */
    break;

  case 6:
    /* TODO */
    break;

  case 7:
    /* TODO */
    break;

  case 8:
    /* TODO */
    break;

  case 9:
    /* TODO */
    break;

  case 0xA:
    /* TODO */
    break;

  case 0xB:
    /* TODO */
    break;

  case 0xC:
    /* TODO */
    break;

  case 0xD:
    /* TODO */
    break;

  case 0xE:
    /* TODO */
    break;

  default:
    /* TODO */
    break;
  }
}

void Processor::execute1100 (void) {
  switch (m_instruction[0] & 0xF) {
  case 0:
  case 1:
    /* TODO */
    break;

  case 2:
    /* TODO */
    break;

  case 3:
    /* TODO */
    break;

  case 4:
    /* TODO */
    break;

  case 5:
    /* TODO */
    break;

  case 6:
  case 7:
    executeMovRmImm ();
    break;

  case 8:
    /* TODO */
    break;

  case 9:
    /* TODO */
    break;

  case 0xA:
    /* TODO */
    break;

  case 0xB:
    /* TODO */
    break;

  case 0xC:
    /* TODO */
    break;

  case 0xD:
    /* TODO */
    break;

  case 0xE:
    /* TODO */
    break;

  default:
    /* TODO */
    break;
  }
}

void Processor::executeMovAccMem (void) {
  Address address;
  address.offset = m_instruction.getWord (1);
  address.segment = m_segmentOverride > -1 ? m_registers.seg[m_segmentOverride] : m_registers.seg[DS];
  bool wide = m_instruction[0] & 1;
  m_registers.setGen (wide, AX, m_memory.read (address, wide));
  m_registers.ip += 3;
}

void Processor::executeMovMemAcc (void) {
  Address address;
  address.offset = m_instruction.getWord (1);
  address.segment = m_segmentOverride > -1 ? m_registers.seg[m_segmentOverride] : m_registers.seg[DS];
  bool wide = m_instruction[0] & 1;
  m_memory.write (address, m_registers.gen[AX], wide);
  m_registers.ip += 3;
}

void Processor::executeMovRegImm (void) {
  bool wide = (m_instruction[0] & 8) != 0;
  m_registers.setGen (wide, m_instruction[0], m_instruction.getByteOrWord (1, wide));
  m_registers.ip += 2 + wide;
}

void Processor::executeMovRegRm (void) {
  bool wide = m_instruction[0] & 1;
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_operand1 = m_memory.read (address, wide);
  } else {
    m_operand1 = m_registers.getGen (wide, m_instruction[1]);
  }
  m_registers.setGen (wide, m_instruction[1] >> 3, m_operand1);
  m_registers.ip += 2;
}

void Processor::executeMovRmImm (void) {
  if ((m_instruction[1] & 0x38) != 0) {
    /* TODO: illegal instruction */
  }
  bool wide = m_instruction[0] & 1;
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_operand1 = m_instruction.getByteOrWord (2 + m_displacementBytes, wide);
    m_memory.write (address, m_operand1, wide);
  } else {
    m_operand1 = m_instruction.getByteOrWord (2, wide);
    m_registers.setGen (wide, m_instruction[1], m_operand1);
  }
  m_registers.ip += 3 + wide;
}

void Processor::executeMovRmReg (void) {
  bool wide = m_instruction[0] & 1;
  m_operand1 = m_registers.getGen (wide, m_instruction[1] >> 3);
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_memory.write (address, m_operand1, wide);
  } else {
    m_registers.setGen (wide, m_instruction[1], m_operand1);
  }
  m_registers.ip += 2;
}

void Processor::executeMovRmSeg (void) {
  if ((m_instruction[1] & 0x20) != 0) {
    /* TODO: illegal instruction */
  }
  int segId = m_instruction[1] >> 3 & 3;
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_memory.writeWord (address, m_registers.seg[segId]);
  } else {
    m_registers.gen[m_instruction[1] & 7] = m_registers.seg[segId];
  }
  m_registers.ip += 2;
}

void Processor::executeMovSegRm (void) {
  if ((m_instruction[1] & 0x20) != 0) {
    /* TODO: illegal instruction */
  }
  int segId = m_instruction[1] >> 3 & 3;
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_operand1 = m_memory.readWord (address);
  } else {
    m_operand1 = m_registers.gen[m_instruction[1] & 7];
  }
  m_registers.seg[segId] = m_operand1;
  m_registers.ip += 2;
}

void Processor::executeNextInstruction (void) {
  Address nextIntructionAddress (m_registers.seg[CS], m_registers.ip);
  m_instruction.read (m_memory, nextIntructionAddress);

  switch (m_instruction[0] >> 4) {
  case 0:
  case 1:
  case 2:
  case 3: /* 00-3F */
    execute00xx ();
    break;

  case 4: /* 40-4F */
    /* TODO */
    break;

  case 5: /* 50-5F */
    /* TODO */
    break;

  case 6: /* 60-6F */
    /* TODO */
    break;

  case 7: /* 70-7F */
    /* TODO */
    break;

  case 8: /* 80-8F */
    execute1000 ();
    break;

  case 9: /* 90-9F */
    execute1001 ();
    break;

  case 10: /* A0-AF */
    execute1010 ();
    break;

  case 11: /* B0-BF */
    executeMovRegImm ();
    break;

  case 12: /* C0-CF */
    execute1100 ();
    break;

  case 13: /* D0-DF */
    /* TODO */
    break;

  case 14: /* E0-EF */
    /* TODO */
    break;

  default: /* F0-FF */
    /* TODO */
    ;
  }
}

Registers& Processor::registers (void) {
  return m_registers;
}

uint8_t Processor::Instruction::operator[] (int index) const {
  return m_array[index];
}

uint16_t Processor::Instruction::getByteOrWord (int index, bool wide) const {
  if (wide) {
    return getWord (index);
  }
  return m_array[index];
}

uint16_t Processor::Instruction::getSignedByteAsWord (int index) const {
  return (int8_t) m_array[index];
}

uint16_t Processor::Instruction::getWord (int index) const {
  uint16_t result = m_array[index + 1];
  result <<= 8;
  result |= m_array[index];
  return result;
}

void Processor::Instruction::read (const Memory& memory, const Address& address) {
  memory.readBytes (m_array, address, 6);
}
