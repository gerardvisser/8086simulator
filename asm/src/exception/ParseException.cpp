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

#include "ParseException.h"

ParseException::ParseException (const char* message, ...) {
  va_list argList;
  va_start (argList, message);
  createMessage (message, argList);
  va_end (argList);
}

ParseException::ParseException (const char* message, va_list args) {
  createMessage (message, args);
  va_end (args);
}

ParseException::ParseException (ParseException&& other) {
  other.m_message = nullptr;
}

ParseException::~ParseException (void) {
}
