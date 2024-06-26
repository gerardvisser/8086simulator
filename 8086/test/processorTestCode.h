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

#ifndef __PROCESSOR_TEST_CODE_INCLUDED
#define __PROCESSOR_TEST_CODE_INCLUDED

#include <cstdint>

namespace processorTestCode {
  extern const uint8_t mov[137];
  extern const uint8_t pushfPopf[2];
  extern const uint8_t pushPop[55];
  extern const uint8_t stos[44];
}

#endif
