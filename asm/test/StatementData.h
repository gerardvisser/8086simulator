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

#ifndef __STATEMENT_DATA_INCLUDED
#define __STATEMENT_DATA_INCLUDED

#include <memory>
#include <string>
#include <vector>

class StatementData {
private:
  std::string m_text;
  int m_size;
  int m_line;

  StatementData (std::string& text, int size, int line);

  static std::shared_ptr<StatementData> convertLine (std::string& text, int lineNo);

public:
  static std::vector<std::shared_ptr<StatementData>> readFromFile (std::string& filename);

  int line (void) const;
  int size (void) const;
  const std::string& text (void) const;
};

#endif
