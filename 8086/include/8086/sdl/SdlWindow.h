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

#ifndef __SDL_WINDOW_INCLUDED
#define __SDL_WINDOW_INCLUDED

#include <string>
#include <SDL2/SDL_video.h>
#include <8086/sdl/Renderer.h>

using std::string;

class SdlWindow {
private:
  SDL_Window* m_window;
  Renderer* m_renderer;

public:
  SdlWindow (string title, int x, int y, int width, int height, int flags = SDL_WINDOW_SHOWN);

  SdlWindow (const SdlWindow&) = delete;
  SdlWindow (SdlWindow&&) = delete;

  ~SdlWindow (void);

  SdlWindow& operator= (const SdlWindow&) = delete;
  SdlWindow& operator= (SdlWindow&&) = delete;

  operator SDL_Window* (void);

  Renderer& renderer (void) const;
};

#endif
