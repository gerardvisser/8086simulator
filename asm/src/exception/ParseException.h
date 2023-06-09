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

#ifndef __PARSE_EXCEPTION_INCLUDED
#define __PARSE_EXCEPTION_INCLUDED

#include "RuntimeException.h"

class ParseException : public RuntimeException {
public:
  explicit ParseException (const char* message, ...);
  ParseException (const char* message, va_list args);

  ParseException (const ParseException&) = delete;
  ParseException (ParseException&& other);

  virtual ~ParseException (void);

  ParseException& operator= (const ParseException&) = delete;
  ParseException& operator= (ParseException&&) = delete;
};

#endif
