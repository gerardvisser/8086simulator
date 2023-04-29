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

#ifndef __MACHINE_INCLUDED
#define __MACHINE_INCLUDED

#include <string>
#include <thread>
#include <8086/VideoCard.h>
#include <8086/sdl/IOTask.h>

using std::string;

class Machine {
private:
  VideoCard m_videoCard;
  Memory m_memory;
  IOTask m_ioTask;
  std::thread m_videoThread;

public:
  explicit Machine (string name);

  Machine (const Machine&) = delete;
  Machine (Machine&&) = delete;

  ~Machine (void);

  Machine& operator= (const Machine&) = delete;
  Machine& operator= (Machine&&) = delete;

  Memory& memory (void);
  VideoCard& videoCard (void);
};

#endif
