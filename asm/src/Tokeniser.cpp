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

#include <cstdint>
#include "Tokeniser.h"

constexpr int eof = std::char_traits<char>::eof ();

Tokeniser::Tokeniser (std::istream& stream, std::map<std::string, TokenData>& tokenData) : m_stream (stream), m_tokenData (tokenData) {
  m_minusPartOfNumber = true;
  m_line = 0;
  m_column = 0;
  m_buffer.reserve (256);
}

std::shared_ptr<Token> Tokeniser::next (void) {
  int c = m_stream.get ();
  if (isWhitespace (c) || c == ';') {
    c = skipWhitespaceAndComments (c);
  }
  if (c == eof) {
    return std::shared_ptr<Token> ();
  }

  std::shared_ptr<Token> token;
  if (isIDFirstChar (c)) {
    token = createIDToken (c);
  } else if (isDigit (c)) {
    token = createNumberToken (c, false);
  } else if (c == '"') {
    token = createStringToken ();
  }
  token = createOtherToken (c);

  Token::Type type = token->type ();
  m_minusPartOfNumber = !(type == Token::Type::SEGREG || type == Token::Type::REG || type == Token::Type::IDENTIFIER ||
                          type == Token::Type::NUMBER || type == Token::Type::RIGHT_PARENTHESIS || type == Token::Type::RIGHT_BRACKET);
  return token;
}

std::shared_ptr<Token> Tokeniser::createHexNumberToken (bool negate) {
  int column = m_column + 2;

  int c = m_stream.get ();
  while (c == '0') {
    ++column;
    c = m_stream.get ();
  }
  while (isHexDigit (c)) {
    m_buffer += c;
    c = m_stream.get ();
  }
  m_stream.unget ();
  column += m_buffer.length ();

  if (column == m_column + 2) {
    Token* token = new Token (Token::Type::NUMBER, m_line, m_column, Token::Error::HEX_NUMBER_WITHOUT_VALUE);
    m_column = column;
    return std::shared_ptr<Token> (token);
  } else if (m_buffer.length () > 8) {
    Token* token = new Token (Token::Type::NUMBER, m_line, m_column, Token::Error::NUMBER_TOO_LARGE);
    m_column = column;
    return std::shared_ptr<Token> (token);
  }

  int i = 0;
  int64_t value = 0;
  while (i < m_buffer.length ()) {
    value <<= 4;
    if (m_buffer[i] > 0x39) {
      value += m_buffer[i] - 0x37;
      if (m_buffer[i] > 0x60) {
        value -= 0x20;
      }
    } else {
      value += m_buffer[i] - 0x30;
    }
    ++i;
  }
  if (negate) {
    value = -value;
  }
  Token* token = new Token (Token::Type::NUMBER, m_line, m_column, value);
  m_column = column;
  return std::shared_ptr<Token> (token);
}

std::shared_ptr<Token> Tokeniser::createIDToken (int c) {
  m_buffer.clear ();
  while (isIDChar (c)) {
    m_buffer += c;
    c = m_stream.get ();
  }
  m_stream.unget ();

  Token* token;
  std::map<std::string, TokenData>::iterator iter = m_tokenData.find (m_buffer);
  if (iter != m_tokenData.end ()) {
    TokenData& tokenData = iter->second;
    token = new Token (tokenData.type (), m_line, m_column, tokenData.subtype ());
  } else {
    token = new Token (Token::Type::IDENTIFIER, m_line, m_column, m_buffer);
  }
  m_column += m_buffer.length ();
  return std::shared_ptr<Token> (token);
}

std::shared_ptr<Token> Tokeniser::createNumberToken (int c, bool negate) {
  int column = m_column;

  m_buffer.clear ();
  if (c == '0') {
    ++column;
    c = m_stream.get ();
    if (c == 'x' || c == 'X') {
      return createHexNumberToken (negate);
    }
  }

  while (c == '0') {
    ++column;
    c = m_stream.get ();
  }
  while (isDigit (c)) {
    m_buffer += c;
    c = m_stream.get ();
  }
  m_stream.unget ();
  column += m_buffer.length ();

  Token* token;
  if (m_buffer.length () > 10) {
    token = new Token (Token::Type::NUMBER, m_line, m_column, Token::Error::NUMBER_TOO_LARGE);
    m_column = column;
    return std::shared_ptr<Token> (token);
  }

  int i = 0;
  int64_t value = 0;
  while (i < m_buffer.length ()) {
    value *= 10;
    value += m_buffer[i] - 0x30;
    ++i;
  }
  if (value > 0x7FFFFFFF) {
    token = new Token (Token::Type::NUMBER, m_line, m_column, Token::Error::NUMBER_TOO_LARGE);
    m_column = column;
    return std::shared_ptr<Token> (token);
  }
  if (negate) {
    value = -value;
  }
  token = new Token (Token::Type::NUMBER, m_line, m_column, value);
  m_column = column;
  return std::shared_ptr<Token> (token);
}

std::shared_ptr<Token> Tokeniser::createOtherToken (int c) {
  /*  als c == '-' && m_minusPartOfNumber && volgende teken is een cijfer: createNumberToken; m_column bijwerken?!? Zie createNumberToken impl.  */
  /* TODO: IMPLEMENT */
}

std::shared_ptr<Token> Tokeniser::createStringToken (void) {
  m_buffer.clear ();
  /* TODO: IMPLEMENT */
}

bool Tokeniser::isDigit (int c) {
  return c >= '0' && c <= '9';
}

bool Tokeniser::isHexDigit (int c) {
  return isDigit (c) || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

bool Tokeniser::isIDChar (int c) {
  return isIDFirstChar (c) || isDigit (c);
}

bool Tokeniser::isIDFirstChar (int c) {
  return c == '_' || isLetter (c);
}

bool Tokeniser::isLetter (int c) {
  return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z';
}

bool Tokeniser::isWhitespace (int c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int Tokeniser::skipWhitespaceAndComments (int c) {
  while (isWhitespace (c) || c == ';') {
    while (isWhitespace (c)) {
      if (c == '\n') {
        ++m_line;
        m_column = 0;
      } else {
        /* Also a tab advances m_column by one.  */
        ++m_column;
      }
      c = m_stream.get ();
    }
    if (c == ';') {
      while (!(c == eof || c == '\n')) {
        c = m_stream.get ();
      }
    }
  }
  return c;
}
