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

#include "tokeniserTest.h"
#include "assertions.h"
#include "../src/Tokeniser.h"
#include "../src/tokenSubtypes.h"
#include <fstream>
#include <vector>

#define checkToken(token, expectedLine, expectedColumn, expectedType, expectedSubtype, expectedError, expectedText) \
    assertTrue (token->line () == expectedLine, "Error in %s: incorrect line, expected=%d, actual=%d\n", #token, expectedLine, token->line ()); \
    assertTrue (token->column () == expectedColumn, "Error in %s: incorrect column, expected=%d, actual=%d\n", #token, expectedColumn, token->column ()); \
    assertTrue (token->type () == expectedType, "Error in %s: incorrect type, expected=%d, actual=%d\n", #token, (int) expectedType, (int) token->type ()); \
    assertTrue (token->subtype () == expectedSubtype, "Error in %s: incorrect subtype, expected=%d, actual=%d\n", #token, expectedSubtype, token->subtype ()); \
    assertTrue (token->error () == expectedError, "Error in %s: incorrect error, expected=%d, actual=%d\n", #token, (int) expectedError, (int) token->error ()); \
    assertTrue (token->text () == expectedText, "Error in %s: incorrect text, expected=%s, actual=%s\n", #token, expectedText, token->text ().c_str ());

static std::vector<std::shared_ptr<Token>> createTokens (std::string filename) {
  std::map<std::string, TokenData> tokenData;
  TokenData::get (tokenData);

  std::ifstream stream (filename);
  Tokeniser tokeniser (stream, tokenData);

  std::vector<std::shared_ptr<Token>> tokens;
  tokens.reserve (256);

  std::shared_ptr<Token> token = tokeniser.next ();
  while (token) {
    tokens.push_back (token);
    token = tokeniser.next ();
  }
  return tokens;
}

void tokeniserTest::next (void) {
  printf ("tokeniserTest::next: ");

  std::vector<std::shared_ptr<Token>> tokens = createTokens ("tokeniserTest.txt");
  assertTrue (tokens.size () == 40, "40 tokens expected, but %ld found.\n", tokens.size ());

  checkToken (tokens[0], 0, 0, Token::Type::SEGREG, TOKEN_SUBTYPE_CS, Token::Error::NONE, "");
  checkToken (tokens[1], 0, 2, Token::Type::COMMA, 0, Token::Error::NONE, "");
  checkToken (tokens[2], 0, 4, Token::Type::NUMBER, 0, Token::Error::NONE, "");
  checkToken (tokens[3], 0, 5, Token::Type::OPERATOR, '-', Token::Error::NONE, "");
  checkToken (tokens[4], 0, 6, Token::Type::NUMBER, 123, Token::Error::NONE, "");
  checkToken (tokens[5], 0, 9, Token::Type::IDENTIFIER, 0, Token::Error::NONE, "fedc");
  checkToken (tokens[6], 0, 13, Token::Type::OPERATOR, '-', Token::Error::NONE, "");
  checkToken (tokens[7], 0, 14, Token::Type::NUMBER, 15, Token::Error::NONE, "");
  checkToken (tokens[8], 0, 17, Token::Type::LEFT_PARENTHESIS, 0, Token::Error::NONE, "");
  checkToken (tokens[9], 0, 18, Token::Type::NUMBER, -19136220, Token::Error::NONE, "");
  checkToken (tokens[10], 0, 28, Token::Type::COLON, 0, Token::Error::NONE, "");
  checkToken (tokens[11], 0, 29, Token::Type::INSTR2, TOKEN_SUBTYPE_XCHG, Token::Error::NONE, "");
  checkToken (tokens[12], 0, 41, Token::Type::NUMBER, -893, Token::Error::NONE, "");
  checkToken (tokens[13], 0, 56, Token::Type::STRING, 0, Token::Error::NONE, "hallo");
  checkToken (tokens[14], 0, 63, Token::Type::STRING, 0, Token::Error::NONE, "");
  checkToken (tokens[15], 0, 65, Token::Type::INSTR1, TOKEN_SUBTYPE_POP, Token::Error::NONE, "");
  checkToken (tokens[16], 1, 3, Token::Type::IDENTIFIER, 0, Token::Error::NONE, "val");
  checkToken (tokens[17], 1, 7, Token::Type::EQUALS, 0, Token::Error::NONE, "");
  checkToken (tokens[18], 1, 9, Token::Type::NUMBER, 21, Token::Error::NONE, "");
  checkToken (tokens[19], 1, 12, Token::Type::OPERATOR, '*', Token::Error::NONE, "");
  checkToken (tokens[20], 1, 14, Token::Type::NUMBER, -3, Token::Error::NONE, "");
  checkToken (tokens[21], 1, 17, Token::Type::OPERATOR, '-', Token::Error::NONE, "");
  checkToken (tokens[22], 1, 18, Token::Type::IDENTIFIER, 0, Token::Error::NONE, "bla_bla6");
  checkToken (tokens[23], 1, 26, Token::Type::OPERATOR, '/', Token::Error::NONE, "");
  checkToken (tokens[24], 1, 27, Token::Type::NUMBER, 0, Token::Error::HEX_NUMBER_WITHOUT_VALUE, "");
  checkToken (tokens[25], 1, 29, Token::Type::LEFT_BRACKET, 0, Token::Error::NONE, "");
  checkToken (tokens[26], 1, 30, Token::Type::RIGHT_BRACKET, 0, Token::Error::NONE, "");
  checkToken (tokens[27], 3, 0, Token::Type::NUMBER, 2147483647, Token::Error::NONE, "");
  checkToken (tokens[28], 3, 10, Token::Type::OPERATOR, '%', Token::Error::NONE, "");
  checkToken (tokens[29], 3, 11, Token::Type::NUMBER, 0, Token::Error::NUMBER_TOO_LARGE, "");
  checkToken (tokens[30], 3, 22, Token::Type::OPERATOR, '+', Token::Error::NONE, "");
  checkToken (tokens[31], 3, 23, Token::Type::NUMBER, -2147483647, Token::Error::NONE, "");
  checkToken (tokens[32], 3, 36, Token::Type::UNDEFINED, 0, Token::Error::STRAY_CHAR, "");
  checkToken (tokens[33], 3, 37, Token::Type::NUMBER, 0, Token::Error::NUMBER_TOO_LARGE, "");
  checkToken (tokens[34], 3, 47, Token::Type::STRING, 0, Token::Error::NONE, "\" string \"\\");
  checkToken (tokens[35], 3, 63, Token::Type::RIGHT_PARENTHESIS, 0, Token::Error::NONE, "");
  checkToken (tokens[36], 3, 64, Token::Type::STRING, 0, Token::Error::STRING_NOT_TERMINATED, "");
  checkToken (tokens[37], 4, 0, Token::Type::OPERATOR, '-', Token::Error::NONE, "");
  checkToken (tokens[38], 4, 1, Token::Type::REG, TOKEN_SUBTYPE_DH, Token::Error::NONE, "");
  checkToken (tokens[39], 4, 3, Token::Type::STRING, 0, Token::Error::STRING_NOT_TERMINATED, "");

  printf ("Ok\n");
}
