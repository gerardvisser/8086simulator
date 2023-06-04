#include "statementCreatorTest.h"
#include "tokeniserTest.h"

int main (int argc, char** args, char** env) {
  tokeniserTest::next ();
  statementCreatorTest::createLabelOrConstantStatement ();
  statementCreatorTest::createInstructionWithOneOperand ();
  statementCreatorTest::createInstructionWithTwoOperands ();
  return 0;
}
