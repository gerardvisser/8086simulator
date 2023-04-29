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

#ifndef __I_O_TASK_INCLUDED
#define __I_O_TASK_INCLUDED

#include <string>
#include <8086/VideoOutputController.h>

using std::string;

class IOTask {
private:
  VideoOutputController& m_videoOutputController;
  const string m_windowTitle;
  bool m_videoThreadToBeStopped;

public:
  IOTask (const string windowTitle, VideoOutputController& videoOutputController);

  IOTask (const IOTask&) = delete;
  IOTask (IOTask&&) = delete;

  ~IOTask (void);

  IOTask& operator= (const IOTask&) = delete;
  IOTask& operator= (IOTask&&) = delete;

  void operator() (void);

  void stopVideoThread (void);
};

#endif
