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

#ifndef __COMPILATION_INCLUDED
#define __COMPILATION_INCLUDED

#include <cstdint>

#ifdef TEST_MODE
# include <memory>
# include <vector>
# include "Statement.h"
#endif

class Compilation {
private:
  uint8_t* m_bytes;
  int m_size;

public:
#ifdef TEST_MODE
  std::vector<std::shared_ptr<Statement>> statements;

  Compilation (int size, std::vector<std::shared_ptr<Statement>>& statements);

#else

  explicit Compilation (int size);

#endif

  Compilation (const Compilation&) = delete;
  Compilation (Compilation&& other);

  Compilation& operator= (const Compilation&) = delete;
  Compilation& operator= (Compilation&& other);

  ~Compilation (void);

  uint8_t* bytes (void) const;
  int size (void) const;
};

#endif
