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

#include <8086/sdl/SdlWindow.h>
#include <8086/sdl/SdlRenderer.h>

SdlWindow::SdlWindow (string title, int x, int y, int width, int height, int flags) {
  m_window = SDL_CreateWindow (title.c_str (), x, y, width, height, flags);
  m_renderer = m_window != nullptr ? new SdlRenderer (*this) : nullptr;
}

SdlWindow::~SdlWindow (void) {
  if (m_renderer != nullptr) {
    delete m_renderer;
  }
  if (m_window != nullptr) {
    SDL_DestroyWindow (m_window);
  }
}

SdlWindow::operator SDL_Window* (void) {
  return m_window;
}

Renderer& SdlWindow::renderer (void) const {
  return *m_renderer;
}
