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

#ifndef __RENDERER_INCLUDED
#define __RENDERER_INCLUDED

#include <8086/sdl/Colour.h>

class Renderer {
public:
  virtual ~Renderer (void);

  virtual void clear (void) = 0;
  virtual void drawPoint (int x, int y) = 0;
  virtual void present (void) = 0;
  virtual void setDrawColour (const Colour& colour) = 0;
  virtual void setResolution (int widthInPixels, int heightInPixels) = 0;
};

#endif
