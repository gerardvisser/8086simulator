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
#include <fstream>
#include "compiler.h"
#include "Config.h"
#include "exception/ParseException.h"

static void writeBinFile (const Compilation& compilation, const std::string& filenameBase);
static void writeCppFile (const Compilation& compilation, const std::string& filenameBase);

int main (int argc, char** args, char** env) {
  Config config (argc, args);
  if (config.errorMessage ()) {
    printf ("%s", config.errorMessage ().value ().c_str ());
    return 1;
  }

  std::ifstream inputStream (config.inputFile ());

  try {

    Compilation compilation = compiler::compile (inputStream, config.startOffset ());
    if (config.binOutput ()) {
      writeBinFile (compilation, config.filenameBase ());
    } else {
      writeCppFile (compilation, config.filenameBase ());
    }

  } catch (ParseException& x) {
    printf ("%s\n", x.message ());
    return 1;
  } catch (RuntimeException& x) {
    printf ("\x1B[31m;RuntimeException: %s\x1B[0m;\n", x.message ());
    return 1;
  }
  return 0;
}

static void writeBinFile (const Compilation& compilation, const std::string& filenameBase) {
  std::string outputFile = filenameBase + ".bin";
  std::ofstream outputStream (outputFile, std::ios::binary);
  if (!outputStream) {
    printf ("Couldn't write file %s.\n", outputFile.c_str ());
    return;
  }
  outputStream.write ((char*) compilation.bytes (), compilation.size ());
}

static void appendNibble (std::string& str, int val) {
  val &= 0xF;
  val += 0x30;
  if (val > 0x39) {
    val += 7;
  }
  str += (char) val;
}

static void appendByte (std::string& str, int val) {
  appendNibble (str, val >> 4);
  appendNibble (str, val);
}

static void appendLine (std::string& str, uint8_t* array, int count) {
  if (count < 1) {
    return;
  }
  str += "\n ";
  for (int i = 0; i < count; ++i) {
    str += " 0x";
    appendByte (str, array[i]);
    str += ',';
  }
}

static void appendLines (std::string& str, uint8_t* array, int count) {
  int q = count / 16;
  int r = count % 16;
  for (int i = 0; i < q; ++i) {
    appendLine (str, array, 16);
    array += 16;
  }
  appendLine (str, array, r);
  str.pop_back ();
}

static void writeCppFile (const Compilation& compilation, const std::string& filenameBase) {
  std::string data;
  data.reserve (6 * compilation.size () + compilation.size () / 8 + 36);
  data += "const uint8_t rom::array[] = {";
  appendLines (data, compilation.bytes (), compilation.size ());
  data += "\n};\n";

  std::string outputFile = filenameBase + ".cpp";
  std::ofstream outputStream (outputFile);
  if (!outputStream) {
    printf ("Couldn't write file %s.\n", outputFile.c_str ());
    return;
  }
  outputStream.write (data.c_str (), data.length ());
}
