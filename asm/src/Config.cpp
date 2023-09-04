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

#include <filesystem>
#include <stdexcept>
#include "Config.h"

static std::string describeUsage (void);

Config::Config (int argc, const char* const * args) : m_binOutput (true) {
  if (argc <= 2 || argc > 4) {
    m_errorMessage = describeUsage ();
    return;
  }

  if (argc == 4) {
    if (!((args[3][0] == 'b' || args[3][0] == 'c') && args[3][1] == 0)) {
      m_errorMessage = "Unknown format specified: only 'b' and 'c' are known.\n";
      return;
    }
    m_binOutput = args[3][0] == 'b';
  }

  std::filesystem::path inputFile = args[1];
  if (!std::filesystem::is_regular_file (inputFile)) {
    m_errorMessage = std::string ("Could not read file ") + args[1] + ".\n";
    return;
  }
  m_inputFile = args[1];

  std::filesystem::path parentDir = inputFile.parent_path ();
  if (parentDir.empty ()) {
    m_filenameBase = inputFile.stem ().string ();
  } else {
    m_filenameBase = parentDir.string () + "/" + inputFile.stem ().string ();
  }

  try {
    std::size_t pos;
    m_startOffset = std::stoi (args[2], &pos, 0);
    if (args[2][pos] != 0) {
      m_errorMessage = "No valid start offset specified.\n";
    } else if (m_startOffset <= -1 || m_startOffset > 0xFFF0) {
      m_errorMessage = "Start offset out of range.\n";
    }
  } catch (std::invalid_argument& x) {
    m_errorMessage = "No valid start offset specified.\n";
  } catch (std::out_of_range& x) {
    m_errorMessage = "Start offset out of range.\n";
  }
}

bool Config::binOutput (void) const {
  return m_binOutput;
}

const std::optional<std::string>& Config::errorMessage (void) const {
  return m_errorMessage;
}

const std::string& Config::filenameBase (void) const {
  return m_filenameBase;
}

const std::string& Config::inputFile (void) const {
  return m_inputFile;
}

int Config::startOffset (void) const {
  return m_startOffset;
}

static std::string describeUsage (void) {
  return std::string ("Usage: asmc [FILE] [START_OFFSET] [OUTPUT_FORMAT]\n") +
      "Compiles 8086 assembly into machine code.  Three arguments can be specified:\n" +
      "the file to compile, the start offset which needs to be an integer, and\n" +
      "optionally the output format.  Two output formats can be specified: b and c.\n" +
      "Format b outputs a binary file with the machine code.  Format c will output a\n" +
      "c++ language file, where the machine code has been converted into an array.\n" +
      "Format b is default.  The name of the output file is based on the name of the\n" +
      "input file: the file extension is removed and replaced by .bin or .cpp.\n" +
      "\nExamples:\n" +
      "./asmc file.txt 0x100\n" +
      "./asmc file.txt 0x100 c\n";
}
