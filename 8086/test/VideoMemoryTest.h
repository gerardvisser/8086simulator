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

#ifndef __VIDEO_MEMORY_TEST_INCLUDED
#define __VIDEO_MEMORY_TEST_INCLUDED

namespace test {
  void readByte_planar_mode0 (void);
  void readByte_planar_mode1 (void);
  void readByte_oddEven (void);
  void readByte_chain4 (void);
  void readByte_memoryMap (void);
  void writeByte_planar_mode0 (void);
  void writeByte_planar_mode1 (void);
  void writeByte_planar_mode2 (void);
  void writeByte_planar_mode3 (void);
  void writeByte_oddEven (void);
  void writeByte_chain4 (void);
  void writeByte_memoryMap (void);
}

#endif
