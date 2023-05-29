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

#ifndef __TOKEN_SUBTYPES_INCLUDED
#define __TOKEN_SUBTYPES_INCLUDED

#define TOKEN_SUBTYPE_ES 0
#define TOKEN_SUBTYPE_CS 1
#define TOKEN_SUBTYPE_SS 2
#define TOKEN_SUBTYPE_DS 3

#define TOKEN_SUBTYPE_AL 0
#define TOKEN_SUBTYPE_CL 1
#define TOKEN_SUBTYPE_DL 2
#define TOKEN_SUBTYPE_BL 3
#define TOKEN_SUBTYPE_AH 4
#define TOKEN_SUBTYPE_CH 5
#define TOKEN_SUBTYPE_DH 6
#define TOKEN_SUBTYPE_BH 7
#define TOKEN_SUBTYPE_AX 8
#define TOKEN_SUBTYPE_CX 9
#define TOKEN_SUBTYPE_DX 10
#define TOKEN_SUBTYPE_BX 11
#define TOKEN_SUBTYPE_SP 12
#define TOKEN_SUBTYPE_BP 13
#define TOKEN_SUBTYPE_SI 14
#define TOKEN_SUBTYPE_DI 15

#define TOKEN_SUBTYPE_DAA   0x27
#define TOKEN_SUBTYPE_DAS   0x2F
#define TOKEN_SUBTYPE_AAA   0x37
#define TOKEN_SUBTYPE_AAS   0x3F
#define TOKEN_SUBTYPE_NOP   0x90
#define TOKEN_SUBTYPE_CBW   0x98
#define TOKEN_SUBTYPE_CWD   0x99
#define TOKEN_SUBTYPE_WAIT  0x9B
#define TOKEN_SUBTYPE_PUSHF 0x9C
#define TOKEN_SUBTYPE_POPF  0x9D
#define TOKEN_SUBTYPE_SAHF  0x9E
#define TOKEN_SUBTYPE_LAHF  0x9F
#define TOKEN_SUBTYPE_MOVSB 0xA4
#define TOKEN_SUBTYPE_MOVSW 0xA5
#define TOKEN_SUBTYPE_CMPSB 0xA6
#define TOKEN_SUBTYPE_CMPSW 0xA7
#define TOKEN_SUBTYPE_STOSB 0xAA
#define TOKEN_SUBTYPE_STOSW 0xAB
#define TOKEN_SUBTYPE_LODSB 0xAC
#define TOKEN_SUBTYPE_LODSW 0xAD
#define TOKEN_SUBTYPE_SCASB 0xAE
#define TOKEN_SUBTYPE_SCASW 0xAF
#define TOKEN_SUBTYPE_INTO  0xCE
#define TOKEN_SUBTYPE_IRET  0xCF
#define TOKEN_SUBTYPE_XLAT  0xD7
#define TOKEN_SUBTYPE_LOCK  0xF0
#define TOKEN_SUBTYPE_REPNZ 0xF2
#define TOKEN_SUBTYPE_REPZ  0xF3
#define TOKEN_SUBTYPE_HLT   0xF4
#define TOKEN_SUBTYPE_CMC   0xF5
#define TOKEN_SUBTYPE_CLC   0xF8
#define TOKEN_SUBTYPE_STC   0xF9
#define TOKEN_SUBTYPE_CLI   0xFA
#define TOKEN_SUBTYPE_STI   0xFB
#define TOKEN_SUBTYPE_CLD   0xFC
#define TOKEN_SUBTYPE_STD   0xFD

#define TOKEN_SUBTYPE_RET   0xC3
#define TOKEN_SUBTYPE_RETF  0xCB
#define TOKEN_SUBTYPE_AAM   0xD4
#define TOKEN_SUBTYPE_AAD   0xD5

#define TOKEN_SUBTYPE_INC   0x40
#define TOKEN_SUBTYPE_DEC   0x48
#define TOKEN_SUBTYPE_PUSH  0x50
#define TOKEN_SUBTYPE_POP   0x58
#define TOKEN_SUBTYPE_NOT   2
#define TOKEN_SUBTYPE_NEG   3
#define TOKEN_SUBTYPE_MUL   4
#define TOKEN_SUBTYPE_IMUL  5
#define TOKEN_SUBTYPE_DIV   6
#define TOKEN_SUBTYPE_IDIV  7

#define TOKEN_SUBTYPE_ADD   0x00
#define TOKEN_SUBTYPE_OR    0x08
#define TOKEN_SUBTYPE_ADC   0x10
#define TOKEN_SUBTYPE_SBB   0x18
#define TOKEN_SUBTYPE_AND   0x20
#define TOKEN_SUBTYPE_SUB   0x28
#define TOKEN_SUBTYPE_XOR   0x30
#define TOKEN_SUBTYPE_CMP   0x38
#define TOKEN_SUBTYPE_MOV   0x8A
#define TOKEN_SUBTYPE_TEST  0x84
#define TOKEN_SUBTYPE_XCHG  0x86
#define TOKEN_SUBTYPE_ROL   0xD0
#define TOKEN_SUBTYPE_ROR   0xD1
#define TOKEN_SUBTYPE_RCL   0xD2
#define TOKEN_SUBTYPE_RCR   0xD3
#define TOKEN_SUBTYPE_SHL   0xD4
#define TOKEN_SUBTYPE_SHR   0xD5
#define TOKEN_SUBTYPE_SAL   0xD6
#define TOKEN_SUBTYPE_SAR   0xD7

#define TOKEN_SUBTYPE_LEA   0x8D
#define TOKEN_SUBTYPE_LES   0xC4
#define TOKEN_SUBTYPE_LDS   0xC5

#define TOKEN_SUBTYPE_CALL   0xE8
#define TOKEN_SUBTYPE_JMP    0xE9

#define TOKEN_SUBTYPE_JO     0x70
#define TOKEN_SUBTYPE_JNO    0x71
#define TOKEN_SUBTYPE_JB     0x72
#define TOKEN_SUBTYPE_JNB    0x73
#define TOKEN_SUBTYPE_JZ     0x74
#define TOKEN_SUBTYPE_JNZ    0x75
#define TOKEN_SUBTYPE_JBE    0x76
#define TOKEN_SUBTYPE_JA     0x77
#define TOKEN_SUBTYPE_JS     0x78
#define TOKEN_SUBTYPE_JNS    0x79
#define TOKEN_SUBTYPE_JPE    0x7A
#define TOKEN_SUBTYPE_JPO    0x7B
#define TOKEN_SUBTYPE_JL     0x7C
#define TOKEN_SUBTYPE_JGE    0x7D
#define TOKEN_SUBTYPE_JLE    0x7E
#define TOKEN_SUBTYPE_JG     0x7F
#define TOKEN_SUBTYPE_LOOPNZ 0xE0
#define TOKEN_SUBTYPE_LOOPZ  0xE1
#define TOKEN_SUBTYPE_LOOP   0xE2
#define TOKEN_SUBTYPE_JCXZ   0xE3

#define TOKEN_SUBTYPE_BYTE 0
#define TOKEN_SUBTYPE_WORD 1

#endif
