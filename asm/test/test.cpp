#include <cstdio>
#include "../src/exception/ParseException.h"
#include "compilerTest.h"
#include "statementCreatorTest.h"
#include "tokeniserTest.h"

int main (int argc, char** args, char** env) {
  try {

    tokeniserTest::next ();
    statementCreatorTest::createLabelOrConstantStatement ();
    statementCreatorTest::createInstructionWithOneOperand ();
    statementCreatorTest::createInstructionWithTwoOperands ();
    statementCreatorTest::createJump ();
    statementCreatorTest::createData ();
    statementCreatorTest::createMiscInstructions ();
    compilerTest::compile ();

  } catch (ParseException& x) {
    printf ("ParseException: %s\n", x.message ());
  } catch (RuntimeException& x) {
    printf ("RuntimeException: %s\n", x.message ());
  }
  return 0;
}
