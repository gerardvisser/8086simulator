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

#include "Compilation.h"

Compilation::Compilation (int size) : m_size (size) {
  m_bytes = size > 0 ? new uint8_t[m_size] : nullptr;
}

Compilation::Compilation (Compilation&& other) : m_size (other.m_size), m_bytes (other.m_bytes) {
  other.m_bytes = nullptr;
  other.m_size = 0;
}

Compilation& Compilation::operator= (Compilation&& other) {
  m_size = other.m_size;
  m_bytes = other.m_bytes;
  other.m_bytes = nullptr;
  other.m_size = 0;
  return *this;
}

Compilation::~Compilation (void) {
  if (m_bytes != nullptr) {
    delete[] m_bytes;
  }
}

uint8_t* Compilation::bytes (void) const {
  return m_bytes;
}

int Compilation::size (void) const {
  return m_size;
}
