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

#include "TokenData.h"
#include "tokenSubtypes.h"

TokenData::TokenData (Token::Type type, int subtype) : m_type (type), m_subtype (subtype) {
}

void TokenData::get (std::map<std::string, TokenData>& tokenData) {
  tokenData["es"] = TokenData (Token::Type::SEGREG, TOKEN_SUBTYPE_ES);
  tokenData["cs"] = TokenData (Token::Type::SEGREG, TOKEN_SUBTYPE_CS);
  tokenData["ss"] = TokenData (Token::Type::SEGREG, TOKEN_SUBTYPE_SS);
  tokenData["ds"] = TokenData (Token::Type::SEGREG, TOKEN_SUBTYPE_DS);
  tokenData["al"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_AL);
  tokenData["cl"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_CL);
  tokenData["dl"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_DL);
  tokenData["bl"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_BL);
  tokenData["ah"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_AH);
  tokenData["ch"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_CH);
  tokenData["dh"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_DH);
  tokenData["bh"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_BH);
  tokenData["ax"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_AX);
  tokenData["cx"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_CX);
  tokenData["dx"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_DX);
  tokenData["bx"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_BX);
  tokenData["sp"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_SP);
  tokenData["bp"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_BP);
  tokenData["si"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_SI);
  tokenData["di"] = TokenData (Token::Type::REG, TOKEN_SUBTYPE_DI);
  tokenData["daa"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_DAA);
  tokenData["das"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_DAS);
  tokenData["aaa"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_AAA);
  tokenData["aas"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_AAS);
  tokenData["nop"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_NOP);
  tokenData["cbw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CBW);
  tokenData["cwd"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CWD);
  tokenData["wait"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_WAIT);
  tokenData["pushf"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_PUSHF);
  tokenData["popf"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_POPF);
  tokenData["sahf"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_SAHF);
  tokenData["lahf"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_LAHF);
  tokenData["movsb"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_MOVSB);
  tokenData["movsw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_MOVSW);
  tokenData["cmpsb"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CMPSB);
  tokenData["cmpsw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CMPSW);
  tokenData["stosb"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_STOSB);
  tokenData["stosw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_STOSW);
  tokenData["lodsb"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_LODSB);
  tokenData["lodsw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_LODSW);
  tokenData["scasb"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_SCASB);
  tokenData["scasw"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_SCASW);
  tokenData["into"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_INTO);
  tokenData["iret"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_IRET);
  tokenData["xlat"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_XLAT);
  tokenData["lock"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_LOCK);
  tokenData["repnz"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_REPNZ);
  tokenData["repz"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_REPZ);
  tokenData["rep"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_REPZ);
  tokenData["hlt"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_HLT);
  tokenData["cmc"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CMC);
  tokenData["clc"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CLC);
  tokenData["stc"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_STC);
  tokenData["cli"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CLI);
  tokenData["sti"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_STI);
  tokenData["cld"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_CLD);
  tokenData["std"] = TokenData (Token::Type::INSTR0, TOKEN_SUBTYPE_STD);
  tokenData["ret"] = TokenData (Token::Type::INSTR01, TOKEN_SUBTYPE_RET);
  tokenData["retf"] = TokenData (Token::Type::INSTR01, TOKEN_SUBTYPE_RETF);
  tokenData["aam"] = TokenData (Token::Type::INSTR01, TOKEN_SUBTYPE_AAM);
  tokenData["aad"] = TokenData (Token::Type::INSTR01, TOKEN_SUBTYPE_AAD);
  tokenData["inc"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_INC);
  tokenData["dec"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_DEC);
  tokenData["push"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_PUSH);
  tokenData["pop"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_POP);
  tokenData["not"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_NOT);
  tokenData["neg"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_NEG);
  tokenData["mul"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_MUL);
  tokenData["imul"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_IMUL);
  tokenData["div"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_DIV);
  tokenData["idiv"] = TokenData (Token::Type::INSTR1, TOKEN_SUBTYPE_IDIV);
  tokenData["int"] = TokenData (Token::Type::INT, 0);
  tokenData["add"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_ADD);
  tokenData["or"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_OR);
  tokenData["adc"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_ADC);
  tokenData["sbb"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SBB);
  tokenData["and"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_AND);
  tokenData["sub"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SUB);
  tokenData["xor"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_XOR);
  tokenData["cmp"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_CMP);
  tokenData["mov"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_MOV);
  tokenData["test"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_TEST);
  tokenData["xchg"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_XCHG);
  tokenData["rol"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_ROL);
  tokenData["ror"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_ROR);
  tokenData["rcl"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_RCL);
  tokenData["rcr"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_RCR);
  tokenData["shl"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SHL);
  tokenData["shr"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SHR);
  tokenData["sal"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SAL);
  tokenData["sar"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_SAR);
  tokenData["lea"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_LEA);
  tokenData["les"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_LES);
  tokenData["lds"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_LDS);
  tokenData["in"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_IN);
  tokenData["out"] = TokenData (Token::Type::INSTR2, TOKEN_SUBTYPE_OUT);
  tokenData["call"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_CALL);
  tokenData["jmp"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JMP);
  tokenData["loopnz"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_LOOPNZ);
  tokenData["loopne"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_LOOPNZ);
  tokenData["loopz"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_LOOPZ);
  tokenData["loope"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_LOOPZ);
  tokenData["loop"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_LOOP);
  tokenData["jcxz"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JCXZ);
  tokenData["jo"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JO);
  tokenData["jno"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNO);
  tokenData["jb"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JB);
  tokenData["jc"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JB);
  tokenData["jnae"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JB);
  tokenData["jnb"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNB);
  tokenData["jnc"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNB);
  tokenData["jae"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNB);
  tokenData["jz"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JZ);
  tokenData["je"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JZ);
  tokenData["jnz"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNZ);
  tokenData["jne"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNZ);
  tokenData["jbe"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JBE);
  tokenData["jna"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JBE);
  tokenData["ja"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JA);
  tokenData["jnbe"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JA);
  tokenData["js"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JS);
  tokenData["jns"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JNS);
  tokenData["jpe"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JPE);
  tokenData["jp"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JPE);
  tokenData["jpo"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JPO);
  tokenData["jnp"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JPO);
  tokenData["jl"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JL);
  tokenData["jnge"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JL);
  tokenData["jge"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JGE);
  tokenData["jnl"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JGE);
  tokenData["jle"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JLE);
  tokenData["jng"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JLE);
  tokenData["jg"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JG);
  tokenData["jnle"] = TokenData (Token::Type::JMP, TOKEN_SUBTYPE_JG);
  tokenData["byte"] = TokenData (Token::Type::PTR_TYPE, TOKEN_SUBTYPE_BYTE);
  tokenData["word"] = TokenData (Token::Type::PTR_TYPE, TOKEN_SUBTYPE_WORD);
  tokenData["ptr"] = TokenData (Token::Type::PTR, 0);
  tokenData["far"] = TokenData (Token::Type::FAR, 0);
  tokenData["db"] = TokenData (Token::Type::DB, 0);
}

int TokenData::subtype (void) const {
  return m_subtype;
}

Token::Type TokenData::type (void) const {
  return m_type;
}
