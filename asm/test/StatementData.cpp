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

#include <cstdlib>
#include <fstream>
#include "StatementData.h"
#include "../src/exception/RuntimeException.h"

StatementData::StatementData (std::string& text, int size, int line) : m_text (text), m_size (size), m_line (line) {
}

std::shared_ptr<StatementData> StatementData::convertLine (std::string& text, int lineNo) {
  int len = text.length ();
  int i = 0;
  while (i < len && text[i] != ';') {
    ++i;
  }
  if (i >= len - 1) {
    return std::shared_ptr<StatementData> ();
  }
  int statementSize = strtol (text.c_str () + i + 1, nullptr, 10);
  return std::shared_ptr<StatementData> (new StatementData (text, statementSize, lineNo));
}

std::vector<std::shared_ptr<StatementData>> StatementData::readFromFile (std::string filename) {
  int lineNo = 0;
  std::string text;
  std::ifstream stream (filename);
  if (!stream) {
    throw RuntimeException ("Could not read file %s.", filename.c_str ());
  }
  std::vector<std::shared_ptr<StatementData>> statementDataList;
  statementDataList.reserve (288);

  while (std::getline (stream, text)) {
    ++lineNo;
    std::shared_ptr<StatementData> statementData = convertLine (text, lineNo);
    if (statementData) {
      statementDataList.push_back (statementData);
    }
  }
  return statementDataList;
}

int StatementData::line (void) const {
  return m_line;
}

int StatementData::size (void) const {
  return m_size;
}

const std::string& StatementData::text (void) const {
  return m_text;
}
