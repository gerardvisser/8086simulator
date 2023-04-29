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

#ifndef __SDL_RENDERER_INCLUDED
#define __SDL_RENDERER_INCLUDED

#include <SDL2/SDL_render.h>
#include <8086/sdl/SdlWindow.h>

class SdlRenderer : public Renderer {
private:
  SDL_Renderer* m_renderer;
  int m_width;
  int m_height;

public:
  SdlRenderer (SdlWindow& window, int index = -1, int flags = 0);

  SdlRenderer (const SdlRenderer&) = delete;
  SdlRenderer (SdlRenderer&&) = delete;

  virtual ~SdlRenderer (void);

  SdlRenderer& operator= (const SdlRenderer&) = delete;
  SdlRenderer& operator= (SdlRenderer&&) = delete;

  void clear (void) override;
  void drawPoint (int x, int y) override;
  void present (void) override;
  void setDrawColour (const Colour& colour) override;
  void setResolution (int widthInPixels, int heightInPixels) override;
};

#endif
