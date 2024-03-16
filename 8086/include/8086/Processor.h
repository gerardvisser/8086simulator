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

#ifndef __PROCESSOR_INCLUDED
#define __PROCESSOR_INCLUDED

#include <8086/Memory.h>
#include <8086/Registers.h>

class Processor {
private:
  class Instruction {
  private:
    uint8_t m_array[6];

  public:
    uint8_t operator[] (int index) const;
    uint16_t getByteOrWord (int index, bool wide) const;
    uint16_t getSignedByteAsWord (int index) const;
    uint16_t getWord (int index) const;
    void read (const Memory& memory, const Address& address);
  };

  Memory& m_memory;
  int64_t m_operand1;
  int64_t m_operand2;
  Registers m_registers;
  Instruction m_instruction;
  int m_segmentOverride;
  int m_displacementBytes;

public:
  explicit Processor (Memory& memory);

  Processor (const Processor&) = delete;
  Processor (Processor&&) = delete;

  ~Processor (void);

  Processor& operator= (const Processor&) = delete;
  Processor& operator= (Processor&&) = delete;

  void executeNextInstruction (void);

  Registers& registers (void);

private:
  Address calculateOperandAddress (void);
  void execute00xx (void);
  void execute00xxx11x (void);
  void execute1000 (void);
  void execute1001 (void);
  void execute1010 (void);
  void execute1100 (void);
  void executeMovAccMem (void);
  void executeMovMemAcc (void);
  void executeMovRegImm (void);
  void executeMovRegRm (void);
  void executeMovRmImm (void);
  void executeMovRmReg (void);
  void executeMovRmSeg (void);
  void executeMovSegRm (void);
/*
  void executeArithmeticTwoOps (void);
*/
};

#endif
