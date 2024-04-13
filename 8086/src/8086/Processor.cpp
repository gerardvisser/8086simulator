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

#define I_DIVIDE_OVERFLOW 0
#define I_TRAP            1
#define I_BREAKPOINT      3
#define I_OVERFLOW        4
#define I_INVALID_OPCODE  6
#define I_NO_COPROCESSOR  7

#define flagSet(flag) ((m_registers.flags & flag) != 0)
#define instructionHasMemoryOperand(modrmByte) (modrmByte >> 6 != 3)

static constexpr uint16_t INTERRUPT_TRAP_CLEAR_MASK = ~(F_TRAP | F_INTERRUPT);

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
    execute00xxx11x ();
    break;

  case 4: /* 00xx x10x */
    /* TODO */
    break;

  default: /* 00xx x0xx */
    /* TODO */
    ;
  }
}

void Processor::execute00xxx11x (void) {
  ++m_registers.ip;
  if ((m_instruction[0] & 0x20) != 0) {
    if ((m_instruction[0] & 1) != 0) {
      /* TODO */
    } else {
      if ((m_instruction[1] & 0xE7) != 0x26) {
        m_segmentOverride = m_instruction[0] >> 3 & 3;
        executeNextInstruction ();
        m_segmentOverride = -1;
      }
    }
  } else {
    executePushPopSeg ();
  }
}

void Processor::execute0110 (void) {
  switch (m_instruction[0] & 0xF) {
  case 8:
    executePushImm ();
    break;

  case 0xA:
    executePushImm8 ();
    break;

  default:
    executeInterrupt (I_INVALID_OPCODE);
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
    executePopRm ();
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
    executePushf ();
    break;

  case 0xD:
    executePopf ();
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
  case 5:
    /* TODO movs */
    break;

  case 6:
  case 7:
    /* TODO cmps */
    break;

  case 8:
    /* TODO */
    break;

  case 9:
    /* TODO */
    break;

  case 0xA:
  case 0xB:
    executeStos ();
    break;

  case 0xC:
  case 0xD:
    /* TODO lods */
    break;

  case 0xE:
  case 0xF:
    /* TODO scas */
    ;
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
    ++m_registers.ip;
    executeInterrupt (I_BREAKPOINT);
    break;

  case 0xD:
    m_registers.ip += 2;
    executeInterrupt (m_instruction[1]);
    break;

  case 0xE:
    /* TODO */
    break;

  default:
    /* TODO */
    break;
  }
}

void Processor::execute1111 (void) {
  switch (m_instruction[0] & 0xF) {
  case 0:
    /* TODO */
    break;

  case 1:
    executeInterrupt (I_INVALID_OPCODE);
    break;

  case 2:
    executeRepnz ();
    break;

  case 3:
    executeRepz ();
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
    executeMiscGroup ();
  }
}

void Processor::executeInterrupt (int id) {
  id <<= 2;
  Address interruptPointerAddress (0, id);

  m_operand1 = m_registers.flags;
  executePush ();
  m_operand1 = m_registers.seg[CS];
  executePush ();
  m_operand1 = m_registers.ip;
  executePush ();
  m_registers.ip = m_memory.readWord (interruptPointerAddress);
  interruptPointerAddress += 2;
  m_registers.seg[CS] = m_memory.readWord (interruptPointerAddress);
  m_registers.flags &= INTERRUPT_TRAP_CLEAR_MASK;
}

void Processor::executeMiscGroup (void) {
  int instructionId = m_instruction[1] >> 3 & 7;
  switch (instructionId) {
  case 0:
    /* TODO: incRm */
    break;

  case 1:
    /* TODO: decRm */
    break;

  case 2:
    /* TODO: callRm */
    break;

  case 3:
    /* TODO: callFarMem */
    break;

  case 4:
    /* TODO: jmpRm */
    break;

  case 5:
    /* TODO: jmpFarMem */
    break;

  case 6:
    executePushRm ();
    break;

  default:
    executeInterrupt (I_INVALID_OPCODE);
    return;
  }
  m_registers.ip += 2;
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
    executeInterrupt (I_INVALID_OPCODE);
    return;
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
    executeInterrupt (I_INVALID_OPCODE);
    return;
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
  int segId = m_instruction[1] >> 3 & 7;
  if (segId == CS || segId > 3) {
    executeInterrupt (I_INVALID_OPCODE);
    return;
  }
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
    executePushPopReg ();
    break;

  case 6: /* 60-6F */
    execute0110 ();
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
    execute1111 ();
  }

  if (flagSet (F_TRAP) && m_segmentOverride == -1) {
    executeInterrupt (I_TRAP);
  }
}

void Processor::executePop (void) {
  m_operand1 = m_memory.readWord (Address (m_registers.seg[SS], m_registers.gen[SP]));
  m_registers.gen[SP] += 2;
}

void Processor::executePopf (void) {
  int64_t mask = 0x0FD5;
  executePop ();
  m_operand1 &= mask;
  mask ^= 0xFFFF;
  m_registers.flags &= mask;
  m_registers.flags |= m_operand1;
  ++m_registers.ip;
}

void Processor::executePopRm (void) {
  if ((m_instruction[1] & 0x38) != 0) {
    executeInterrupt (I_INVALID_OPCODE);
    return;
  }
  executePop ();
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_memory.writeWord (address, m_operand1);
  } else {
    m_registers.gen[m_instruction[1] & 7] = m_operand1;
  }
  m_registers.ip += 2;
}

void Processor::executePush (void) {
  m_registers.gen[SP] -= 2;
  m_memory.writeWord (Address (m_registers.seg[SS], m_registers.gen[SP]), m_operand1);
}

void Processor::executePushf (void) {
  m_operand1 = m_registers.flags;
  executePush ();
  ++m_registers.ip;
}

void Processor::executePushImm (void) {
  m_operand1 = m_instruction.getWord (1);
  executePush ();
  m_registers.ip += 3;
}

void Processor::executePushImm8 (void) {
  m_operand1 = m_instruction.getSignedByteAsWord (1);
  executePush ();
  m_registers.ip += 2;
}

void Processor::executePushPopReg (void) {
  if ((m_instruction[0] & 0x08) != 0) {
    executePop ();
    m_registers.gen[m_instruction[0] & 7] = m_operand1;
  } else {
    m_operand1 = m_registers.gen[m_instruction[0] & 7];
    executePush ();
  }
  ++m_registers.ip;
}

void Processor::executePushPopSeg (void) {
  int segId = m_instruction[0] >> 3;
  if ((m_instruction[0] & 1) != 0) {
    if (segId == CS) {
      --m_registers.ip;
      executeInterrupt (I_INVALID_OPCODE);
      return;
    }
    executePop ();
    m_registers.seg[segId] = m_operand1;
  } else {
    m_operand1 = m_registers.seg[segId];
    executePush ();
  }
}

void Processor::executePushRm (void) {
  if (instructionHasMemoryOperand (m_instruction[1])) {
    Address address = calculateOperandAddress ();
    m_operand1 = m_memory.readWord (address);
  } else {
    m_operand1 = m_registers.gen[m_instruction[1] & 7];
  }
  executePush ();
}

void Processor::executeRepnz (void) {
  switch ((uint8_t) (m_instruction[1] - 0xA4)) {
  case 0:
  case 1:
    /* TODO movs */
    break;

  case 2:
  case 3:
    /* TODO cmps */
    break;

  case 6:
  case 7:
    executeRepStos ();
    break;

  case 8:
  case 9:
    /* TODO lods */
    break;

  case 10:
  case 11:
    /* TODO scas */
    break;

  default:
    ++m_registers.ip;
  }
}

void Processor::executeRepStos (void) {
  if (m_registers.gen[CX] > 0) {
    Address address (m_registers.seg[ES], m_registers.gen[DI]);
    bool wide = m_instruction[1] & 1;
    int step = 1 + wide;
    if (flagSet (F_DIRECTION)) {
      step = -step;
    }

    if (flagSet (F_TRAP)) {
        m_memory.write (address, m_registers.gen[AX], wide);
        m_registers.gen[DI] += step;
        --m_registers.gen[CX];
        if (m_registers.gen[CX] == 0) {
          m_registers.ip += 2;
        }
    } else {
      do {
        m_memory.write (address, m_registers.gen[AX], wide);
        m_registers.gen[DI] += step;
        address += step;
        --m_registers.gen[CX];
      } while (m_registers.gen[CX] > 0);
      m_registers.ip += 2;
    }
  } else {
    m_registers.ip += 2;
  }
}

void Processor::executeRepz (void) {
  switch ((uint8_t) (m_instruction[1] - 0xA4)) {
  case 0:
  case 1:
    /* TODO movs */
    break;

  case 2:
  case 3:
    /* TODO cmps */
    break;

  case 6:
  case 7:
    executeRepStos ();
    break;

  case 8:
  case 9:
    /* TODO lods */
    break;

  case 10:
  case 11:
    /* TODO scas */
    break;

  default:
    ++m_registers.ip;
  }
}

void Processor::executeStos (void) {
  Address address (m_registers.seg[ES], m_registers.gen[DI]);
  bool wide = m_instruction[0] & 1;
  int step = 1 + wide;
  m_memory.write (address, m_registers.gen[AX], wide);
  if (flagSet (F_DIRECTION)) {
    m_registers.gen[DI] -= step;
  } else {
    m_registers.gen[DI] += step;
  }
  ++m_registers.ip;
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
