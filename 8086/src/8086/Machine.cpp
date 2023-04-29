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

#include <8086/Machine.h>

Machine::Machine (string name) : m_memory (m_videoCard.videoMemory ()), m_ioTask (name, m_videoCard.videoOutputController ()) {
  m_videoCard.loadRom (m_memory);
  m_videoThread = std::move (std::thread (std::ref (m_ioTask)));
}

Machine::~Machine (void) {
  m_ioTask.stopVideoThread ();
  if (m_videoThread.joinable ()) {
    m_videoThread.join ();
  }
}

Memory& Machine::memory (void) {
  return m_memory;
}

VideoCard& Machine::videoCard (void) {
  return m_videoCard;
}
