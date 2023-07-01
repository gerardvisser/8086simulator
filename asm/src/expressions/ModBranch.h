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

#ifndef __MOD_BRANCH_INCLUDED
#define __MOD_BRANCH_INCLUDED

#include "Branch.h"

class ModBranch : public Branch {
public:
  ModBranch (void);

  ModBranch (const ModBranch&) = delete;
  ModBranch (ModBranch&&) = delete;

  virtual ~ModBranch (void);

  ModBranch& operator= (const ModBranch&) = delete;
  ModBranch& operator= (ModBranch&&) = delete;

  int64_t value (void) const override;
};

#endif
