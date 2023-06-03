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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "RuntimeException.h"

RuntimeException::RuntimeException (void) {
}

RuntimeException::RuntimeException (const char* message, ...) {
  va_list argList;
  va_start (argList, message);
  createMessage (message, argList);
  va_end (argList);
}

RuntimeException::RuntimeException (RuntimeException&& other) : m_message (other.m_message) {
  other.m_message = nullptr;
}

RuntimeException::~RuntimeException (void) {
  if (m_message != nullptr) {
    free (m_message);
  }
}

void RuntimeException::createMessage (const char* message, va_list args) {
  int size = strlen (message) + 256;
  m_message = (char*) malloc (size);

  va_list argsCopy;
  va_copy (argsCopy, args);
  int actualSize = vsnprintf (m_message, size, message, args);
  if (actualSize >= size) {
    size = actualSize + 1;
    m_message = (char*) realloc (m_message, size);
    vsnprintf (m_message, size, message, argsCopy);
  }
  va_end (argsCopy);
}

const char* RuntimeException::message (void) {
  return m_message;
}
