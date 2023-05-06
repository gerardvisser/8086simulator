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

#include <8086/sdl/IOTask.h>
#include <8086/sdl/SdlWindow.h>
#include <thread>
#include <SDL2/SDL.h>

using namespace std::chrono_literals;

IOTask::IOTask (const string windowTitle, VideoOutputController& videoOutputController) : m_windowTitle (windowTitle), m_videoOutputController (videoOutputController) {
}

IOTask::~IOTask (void) {
}

void IOTask::operator() (void) {
  m_videoThreadToBeStopped = false;
  SDL_Init (SDL_INIT_VIDEO);

  {
    SdlWindow window (m_windowTitle, 0, 0, 1440, 960);
    Renderer& renderer = window.renderer ();

    while (!m_videoThreadToBeStopped) {
      m_videoOutputController.drawScreen (renderer);
      std::this_thread::sleep_for (250ms);
    }
  }

  SDL_Quit ();
}

void IOTask::stopVideoThread (void) {
  m_videoThreadToBeStopped = true;
}
