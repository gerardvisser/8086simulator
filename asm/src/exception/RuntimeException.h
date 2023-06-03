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

#ifndef __RUNTIME_EXCEPTION_INCLUDED
#define __RUNTIME_EXCEPTION_INCLUDED

#include <cstdarg>

class RuntimeException {
protected:
  char* m_message;

  RuntimeException (void);
  void createMessage (const char* message, va_list args);

public:
  RuntimeException (const char* message, ...);

  RuntimeException (const RuntimeException&) = delete;
  RuntimeException (RuntimeException&& other);

  virtual ~RuntimeException (void);

  RuntimeException& operator= (const RuntimeException&) = delete;
  RuntimeException& operator= (RuntimeException&&) = delete;

  const char* message (void);
};

#endif
