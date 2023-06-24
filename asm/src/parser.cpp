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

#include "parser.h"
#include "statementCreator.h"
#include "Tokeniser.h"
#include "exception/ParseException.h"

static void appendErrorMessage (std::string& errorMessages, std::shared_ptr<Token>& token);

std::vector<std::shared_ptr<Statement>> parser::parse (std::istream& stream) {
  std::map<std::string, TokenData> tokenData;
  TokenData::get (tokenData);

  Tokeniser tokeniser (stream, tokenData);

  std::vector<std::shared_ptr<Token>> tokens;
  tokens.reserve (65536);

  std::string errorMessages;
  errorMessages.reserve (256);

  std::shared_ptr<Token> token = tokeniser.next ();
  while (token) {
    if (token->error () != Token::Error::NONE) {
      appendErrorMessage (errorMessages, token);
    }
    tokens.push_back (token);
    token = tokeniser.next ();
  }

  if (!errorMessages.empty ()) {
    throw ParseException (errorMessages.c_str ());
  }
  return statementCreator::create (tokens);
}

static void appendErrorMessage (std::string& errorMessages, std::shared_ptr<Token>& token) {
  if (!errorMessages.empty ()) {
    errorMessages += '\n';
  }
  errorMessages += "Error in line ";
  errorMessages += std::to_string (token->line ());
  errorMessages += ", column ";
  errorMessages += std::to_string (token->column ());
  errorMessages += ": ";
  switch (token->error ()) {
  case Token::Error::HEX_NUMBER_WITHOUT_VALUE:
    errorMessages += "hex. number without a value.";
    break;

  case Token::Error::NUMBER_TOO_LARGE:
    errorMessages += "number too large.";
    break;

  case Token::Error::STRING_NOT_TERMINATED:
    errorMessages += "string not terminated.";
    break;

  case Token::Error::STRAY_CHAR:
    errorMessages += "stray character.";
    break;

  default:
    errorMessages += "unknown error: this shouldn't occur.";
  }
}
