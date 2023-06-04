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

#include "statementCreatorTest.h"
#include "assertions.h"
#include "testUtils.h"
#include "../src/statementCreator.h"
#include "../src/tokenSubtypes.h"
//#include "../src/exception/ParseException.h"

void statementCreatorTest::createInstructionWithOneOperand (void) {
  printf ("statementCreatorTest::createInstructionWithOneOperand: ");

  std::string text = std::string ("inc bx\n") +
      "push cs\n" +
      "dec ah\n" +
      "push a + 1\n" +
      "mul [bx + si]\n" +
      "div [di + bx]\n" +
      "neg word ptr [si + bp]\n" +
      "not byte ptr [bp + di]\n" +
      "idiv [si]\n" +
      "imul [di]\n" +
      "inc word ptr [bp]\n" +
      "inc word ptr [0x8000]\n" +
      "inc byte ptr [bx]\n" +
      "mul [si + bx + 4 * 5]\n" +
      "div [bx + di + 23 + x]\n" +
      "neg word ptr [bp + si + -1]\n" +
      "not byte ptr [di + bp + (3)]\n" +
      "pop [si + 0]\n" +
      "imul [di + 0x20]\n" +
      "inc word ptr [bp + c2]\n" +
      "inc byte ptr [bx + c4/8]\n";

  std::vector<std::shared_ptr<Token>> tokens = testUtils::createTokensFromString (text);
  std::vector<std::shared_ptr<Statement>> statements = statementCreator::create (tokens);
  assertTrue (statements.size () == 21, "(line %d) twenty one statements expected (%ld created)\n", __LINE__, statements.size ());

  //inc bx
  assertTrue (statements[0]->type () == Statement::Type::INSTRUCTION, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[0]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[0]->tokenCount () == 2, "(line %d) two tokens expected for statement\n", __LINE__);
  assertTrue (statements[0]->token (0)->type () == Token::Type::INSTR1, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[0]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[0]->token (1)->type () == Token::Type::REG, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[0]->token (1)->subtype () == TOKEN_SUBTYPE_BX, "(line %d) wrong token subtype\n", __LINE__);
  Operand operand = statements[0]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 3, "(line %d) wrong operand id\n", __LINE__);

  //push cs
  assertTrue (statements[1]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[1]->tokenCount () == 2, "(line %d) two tokens expected for statement\n", __LINE__);
  assertTrue (statements[1]->token (0)->type () == Token::Type::INSTR1, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[1]->token (0)->subtype () == TOKEN_SUBTYPE_PUSH, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[1]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::SEGMENT_REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 1, "(line %d) wrong operand id\n", __LINE__);

  //dec ah
  assertTrue (statements[2]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[2]->tokenCount () == 2, "(line %d) two tokens expected for statement\n", __LINE__);
  assertTrue (statements[2]->token (0)->subtype () == TOKEN_SUBTYPE_DEC, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[2]->operand (0);
  assertTrue (operand.width () == Operand::Width::BYTE, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 4, "(line %d) wrong operand id\n", __LINE__);

  //push a + 1
  assertTrue (statements[3]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[3]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[3]->token (0)->subtype () == TOKEN_SUBTYPE_PUSH, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[3]->token (1)->text () == "a", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[3]->token (2)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[3]->token (3)->subtype () == 1, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[3]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::IMMEDIATE, "(line %d) wrong operand type\n", __LINE__);

  //mul [bx + si]
  assertTrue (statements[4]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[4]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[4]->token (0)->subtype () == TOKEN_SUBTYPE_MUL, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[4]->token (1)->type () == Token::Type::LEFT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[4]->token (2)->type () == Token::Type::REG, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[4]->token (3)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[4]->token (4)->type () == Token::Type::REG, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[4]->token (5)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[4]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0, "(line %d) wrong operand id\n", __LINE__);

  //div [di + bx]
  assertTrue (statements[5]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[5]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[5]->token (0)->subtype () == TOKEN_SUBTYPE_DIV, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[5]->token (5)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[5]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 1, "(line %d) wrong operand id\n", __LINE__);

  //neg word ptr [si + bp]
  assertTrue (statements[6]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[6]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[6]->token (0)->subtype () == TOKEN_SUBTYPE_NEG, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[6]->token (5)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[6]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 2, "(line %d) wrong operand id\n", __LINE__);

  //not byte ptr [bp + di]
  assertTrue (statements[7]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[7]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[7]->token (0)->subtype () == TOKEN_SUBTYPE_NOT, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[7]->operand (0);
  assertTrue (operand.width () == Operand::Width::BYTE, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 3, "(line %d) wrong operand id\n", __LINE__);

  //idiv [si]
  assertTrue (statements[8]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[8]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[8]->token (0)->subtype () == TOKEN_SUBTYPE_IDIV, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[8]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 4, "(line %d) wrong operand id\n", __LINE__);

  //imul [di]
  assertTrue (statements[9]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[9]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[9]->token (0)->subtype () == TOKEN_SUBTYPE_IMUL, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[9]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 5, "(line %d) wrong operand id\n", __LINE__);

  //inc word ptr [bp] => inc word ptr [bp+0]
  assertTrue (statements[10]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[10]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[10]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[10]->token (1)->type () == Token::Type::LEFT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[10]->token (2)->type () == Token::Type::REG, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[10]->token (3)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[10]->token (4)->type () == Token::Type::NUMBER, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[10]->token (4)->subtype () == 0, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[10]->token (5)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[10]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xE, "(line %d) wrong operand id\n", __LINE__);

  //inc word ptr [0x8000]
  assertTrue (statements[11]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[11]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[11]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[11]->token (1)->type () == Token::Type::LEFT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[11]->token (2)->type () == Token::Type::NUMBER, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[11]->token (2)->subtype () == 0x8000, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[11]->token (3)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[11]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 6, "(line %d) wrong operand id\n", __LINE__);

  //inc byte ptr [bx]
  assertTrue (statements[12]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[12]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[12]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[12]->operand (0);
  assertTrue (operand.width () == Operand::Width::BYTE, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 7, "(line %d) wrong operand id\n", __LINE__);

  //mul [si + bx + 4 * 5]
  assertTrue (statements[13]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[13]->tokenCount () == 10, "(line %d) ten tokens expected for statement\n", __LINE__);
  assertTrue (statements[13]->token (0)->subtype () == TOKEN_SUBTYPE_MUL, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (1)->type () == Token::Type::LEFT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[13]->token (2)->subtype () == TOKEN_SUBTYPE_SI, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (3)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (4)->subtype () == TOKEN_SUBTYPE_BX, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (5)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (6)->subtype () == 4, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (7)->subtype () == '*', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (8)->subtype () == 5, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[13]->token (9)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[13]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 8, "(line %d) wrong operand id\n", __LINE__);

  //div [bx + di + 23 + x]
  assertTrue (statements[14]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[14]->tokenCount () == 10, "(line %d) ten tokens expected for statement\n", __LINE__);
  assertTrue (statements[14]->token (0)->subtype () == TOKEN_SUBTYPE_DIV, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[14]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 9, "(line %d) wrong operand id\n", __LINE__);

  //neg word ptr [bp + si + -1]
  assertTrue (statements[15]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[15]->tokenCount () == 8, "(line %d) eight tokens expected for statement\n", __LINE__);
  assertTrue (statements[15]->token (0)->subtype () == TOKEN_SUBTYPE_NEG, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[15]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xA, "(line %d) wrong operand id\n", __LINE__);

  //not byte ptr [di + bp + (3)]
  assertTrue (statements[16]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[16]->tokenCount () == 10, "(line %d) ten tokens expected for statement\n", __LINE__);
  assertTrue (statements[16]->token (0)->subtype () == TOKEN_SUBTYPE_NOT, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[16]->operand (0);
  assertTrue (operand.width () == Operand::Width::BYTE, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xB, "(line %d) wrong operand id\n", __LINE__);

  //pop [si + 0]
  assertTrue (statements[17]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[17]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[17]->token (0)->subtype () == TOKEN_SUBTYPE_POP, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[17]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xC, "(line %d) wrong operand id\n", __LINE__);

  //imul [di + 0x20]
  assertTrue (statements[18]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[18]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[18]->token (0)->subtype () == TOKEN_SUBTYPE_IMUL, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[18]->operand (0);
  assertTrue (operand.width () == Operand::Width::UNDEFINED, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xD, "(line %d) wrong operand id\n", __LINE__);

  //inc word ptr [bp + c2]
  assertTrue (statements[19]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[19]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[19]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[19]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xE, "(line %d) wrong operand id\n", __LINE__);

  //inc byte ptr [bx + c4/8]
  assertTrue (statements[20]->operandCount () == 1, "(line %d) operand count not 1\n", __LINE__);
  assertTrue (statements[20]->tokenCount () == 8, "(line %d) eight tokens expected for statement\n", __LINE__);
  assertTrue (statements[20]->token (0)->subtype () == TOKEN_SUBTYPE_INC, "(line %d) wrong token subtype\n", __LINE__);
  operand = statements[20]->operand (0);
  assertTrue (operand.width () == Operand::Width::BYTE, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0xF, "(line %d) wrong operand id\n", __LINE__);

  printf ("Ok\n");
}

void statementCreatorTest::createInstructionWithTwoOperands (void) {
  printf ("statementCreatorTest::createInstructionWithTwoOperands: ");

  /* TODO: Meer maken */
  std::string text = std::string ("mov es, ax\n") +
      "lea cx, [bx + di]\n";

  std::vector<std::shared_ptr<Token>> tokens = testUtils::createTokensFromString (text);
  std::vector<std::shared_ptr<Statement>> statements = statementCreator::create (tokens);
  assertTrue (statements.size () == 2, "(line %d) two statements expected (%ld created)\n", __LINE__, statements.size ());

  //mov es, ax
  assertTrue (statements[0]->type () == Statement::Type::INSTRUCTION, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[0]->operandCount () == 2, "(line %d) operand count not 2\n", __LINE__);
  assertTrue (statements[0]->tokenCount () == 4, "(line %d) four tokens expected for statement\n", __LINE__);
  assertTrue (statements[0]->token (0)->type () == Token::Type::INSTR2, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[0]->token (0)->subtype () == TOKEN_SUBTYPE_MOV, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[0]->token (1)->type () == Token::Type::SEGREG, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[0]->token (1)->subtype () == TOKEN_SUBTYPE_ES, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[0]->token (2)->type () == Token::Type::COMMA, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[0]->token (3)->subtype () == TOKEN_SUBTYPE_AX, "(line %d) wrong token subtype\n", __LINE__);
  Operand operand = statements[0]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::SEGMENT_REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0, "(line %d) wrong operand id\n", __LINE__);
  operand = statements[0]->operand (1);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 0, "(line %d) wrong operand id\n", __LINE__);

  //lea cx, [bx + di]
  assertTrue (statements[1]->operandCount () == 2, "(line %d) operand count not 2\n", __LINE__);
  assertTrue (statements[1]->tokenCount () == 8, "(line %d) eight tokens expected for statement\n", __LINE__);
  assertTrue (statements[1]->token (0)->type () == Token::Type::LOAD, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[1]->token (0)->subtype () == TOKEN_SUBTYPE_LEA, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[1]->token (1)->subtype () == TOKEN_SUBTYPE_CX, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[1]->token (2)->type () == Token::Type::COMMA, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[1]->token (3)->type () == Token::Type::LEFT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[1]->token (4)->subtype () == TOKEN_SUBTYPE_BX, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[1]->token (5)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[1]->token (6)->subtype () == TOKEN_SUBTYPE_DI, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[1]->token (7)->type () == Token::Type::RIGHT_BRACKET, "(line %d) wrong token type\n", __LINE__);
  operand = statements[1]->operand (0);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::REGISTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 1, "(line %d) wrong operand id\n", __LINE__);
  operand = statements[1]->operand (1);
  assertTrue (operand.width () == Operand::Width::WORD, "(line %d) wrong operand width\n", __LINE__);
  assertTrue (operand.type () == Operand::Type::POINTER, "(line %d) wrong operand type\n", __LINE__);
  assertTrue (operand.id () == 1, "(line %d) wrong operand id\n", __LINE__);

  printf ("Ok\n");
}

void statementCreatorTest::createLabelOrConstantStatement (void) {
  printf ("statementCreatorTest::createLabelOrConstantStatement: ");

  std::string text = std::string ("label1:\n") +
      "c1 = 5\n" +
      "c2 = -6 * c1 + c1\n" +
      "c3 = c2 * (3 + c1)\n" +
      "   label2:\n" +
      "c4 = (c2 - c3 / (c1 + c5 % -7))";

  std::vector<std::shared_ptr<Token>> tokens = testUtils::createTokensFromString (text);
  std::vector<std::shared_ptr<Statement>> statements = statementCreator::create (tokens);
  assertTrue (statements.size () == 6, "(line %d) six statements expected\n", __LINE__);

  assertTrue (statements[0]->type () == Statement::Type::LABEL, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[0]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[0]->tokenCount () == 1, "(line %d) one token expected for statement\n", __LINE__);
  assertTrue (statements[0]->token (0)->text () == "label1", "(line %d) wrong token text\n", __LINE__);

  assertTrue (statements[1]->type () == Statement::Type::CONSTANT, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[1]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[1]->tokenCount () == 2, "(line %d) two tokens expected for statement\n", __LINE__);
  assertTrue (statements[1]->token (0)->text () == "c1", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[1]->token (1)->subtype () == 5, "(line %d) wrong token subtype\n", __LINE__);

  assertTrue (statements[2]->type () == Statement::Type::CONSTANT, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[2]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[2]->tokenCount () == 6, "(line %d) six tokens expected for statement\n", __LINE__);
  assertTrue (statements[2]->token (0)->text () == "c2", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[2]->token (1)->subtype () == -6, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[2]->token (2)->subtype () == '*', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[2]->token (3)->text () == "c1", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[2]->token (4)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[2]->token (5)->text () == "c1", "(line %d) wrong token text\n", __LINE__);

  assertTrue (statements[3]->type () == Statement::Type::CONSTANT, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[3]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[3]->tokenCount () == 8, "(line %d) eight tokens expected for statement\n", __LINE__);
  assertTrue (statements[3]->token (0)->text () == "c3", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[3]->token (1)->text () == "c2", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[3]->token (2)->subtype () == '*', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[3]->token (3)->type () == Token::Type::LEFT_PARENTHESIS, "(line %d) wrong token type\n", __LINE__);
  assertTrue (statements[3]->token (4)->subtype () == 3, "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[3]->token (5)->subtype () == '+', "(line %d) wrong token subtype\n", __LINE__);
  assertTrue (statements[3]->token (6)->text () == "c1", "(line %d) wrong token text\n", __LINE__);
  assertTrue (statements[3]->token (7)->type () == Token::Type::RIGHT_PARENTHESIS, "(line %d) wrong token type\n", __LINE__);

  assertTrue (statements[4]->type () == Statement::Type::LABEL, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[4]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[4]->tokenCount () == 1, "(line %d) one token expected for statement\n", __LINE__);
  assertTrue (statements[4]->token (0)->text () == "label2", "(line %d) wrong token text\n", __LINE__);

  assertTrue (statements[5]->type () == Statement::Type::CONSTANT, "(line %d) wrong statement type\n", __LINE__);
  assertTrue (statements[5]->operandCount () == 0, "(line %d) operand count not 0\n", __LINE__);
  assertTrue (statements[5]->tokenCount () == 14, "(line %d) eight tokens expected for statement\n", __LINE__);
  assertTrue (statements[5]->token (0)->text () == "c4", "(line %d) wrong token text\n", __LINE__);

  printf ("Ok\n");
}
