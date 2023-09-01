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
#include "assertions.h"
#include "compilerTest.h"
#include "StatementData.h"
#include "../src/compiler.h"

#define EXPECTED_BYTE_BUFFER_SIZE 32

class StatementIter {
private:
  std::vector<std::shared_ptr<Statement>>::iterator m_statements;
  const std::vector<std::shared_ptr<Statement>>::iterator m_statementsEnd;

public:
  explicit StatementIter (std::vector<std::shared_ptr<Statement>>& statements) : m_statements (statements.begin ()), m_statementsEnd (statements.end ()) {
    findNext ();
  }

  StatementIter& operator++ (void) {
    ++m_statements;
    findNext ();
    return *this;
  }

  std::shared_ptr<Statement> operator* (void) {
    if (m_statements == m_statementsEnd) {
      return std::shared_ptr<Statement> ();
    }
    return *m_statements;
  }

private:
  void findNext (void) {
    while (!(m_statements == m_statementsEnd || (*m_statements)->type () == Statement::Type::INSTRUCTION || (*m_statements)->type () == Statement::Type::DATA)) {
      ++m_statements;
    }
  }
};

static bool readNextExpectedBytes (uint8_t* expected, std::ifstream& expectedBytes, int count) {
  expectedBytes.read ((char*) expected, count);
  return expectedBytes.gcount () == count;
}

void compilerTest::compile (void) {
  printf ("compilerTest::compile: ");

  uint8_t expected[EXPECTED_BYTE_BUFFER_SIZE];
  std::ifstream expectedBytes ("compilerTest.bin", std::ios::binary);
  assertTrue (expectedBytes, "Failed: could not read file compilerTest.bin\n");
  std::vector<std::shared_ptr<StatementData>> statementDataList = StatementData::readFromFile ("compilerTest.txt");
  std::ifstream input ("compilerTest.txt");

  Compilation compilation = compiler::compile (input, 0x100);

  int i = 0;
  StatementIter statements (compilation.statements);

  for (std::shared_ptr<StatementData> statementData : statementDataList) {
    std::shared_ptr<Statement> statement = *statements;
    assertTrue (statement, "No statement found for line %d (%s)\n", statementData->line (), statementData->text ().c_str ());
    assertTrue (statement->size () == statementData->size (), "Wrong size (%d) found for statement in line %d (%s), expected %d.\n",
        statement->size (), statementData->line (), statementData->text ().c_str (), statementData->size ());
    assertTrue (readNextExpectedBytes (expected, expectedBytes, statementData->size ()),
        "Couldn't read the expected bytes for statement in line %d.\n", statementData->line ());
    assertTrue (i + statementData->size () <= compilation.size (),
        "Couldn't read the actual bytes for statement in line %d.\n", statementData->line ());
    for (int j = 0; j < statementData->size (); ++j) {

      if (expected[j] != compilation.bytes ()[i + j]) {
        printf ("An error will occur (offset=0x%X)\n", i);
        for (int k = 0; k < statementData->size (); ++k) {
          printf ("  index:%d, expected: %02X, actual: %02X\n", k, expected[k], compilation.bytes ()[i + k]);
        }
      }

      assertTrue (expected[j] == compilation.bytes ()[i + j], "  Compilation incorrect for line %d (%s).\n", statementData->line (), statementData->text ().c_str ());

    }
    i += statementData->size ();
    ++statements;
  }
  printf ("Ok\n");
}
