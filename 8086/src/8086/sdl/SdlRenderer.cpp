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

#include <8086/sdl/SdlRenderer.h>

SdlRenderer::SdlRenderer (SdlWindow& window, int index, int flags) {
  m_renderer = SDL_CreateRenderer (window, index, flags);
  SDL_GetWindowSize (window, &m_width, &m_height);
}

SdlRenderer::~SdlRenderer (void) {
  if (m_renderer != nullptr) {
    SDL_DestroyRenderer (m_renderer);
  }
}

void SdlRenderer::clear (void) {
  SDL_RenderClear (m_renderer);
}

void SdlRenderer::drawPoint (int x, int y) {
  SDL_RenderDrawPoint (m_renderer, x, y);
}

void SdlRenderer::present (void) {
  SDL_RenderPresent (m_renderer);
}

void SdlRenderer::setDrawColour (const Colour& colour) {
  SDL_SetRenderDrawColor (m_renderer, colour.red, colour.green, colour.blue, SDL_ALPHA_OPAQUE);
}

void SdlRenderer::setResolution (int widthInPixels, int heightInPixels) {
  double scaleX = (double) m_width / widthInPixels;
  double scaleY = (double) m_height / heightInPixels;
  SDL_RenderSetScale (m_renderer, scaleX, scaleY);
}
