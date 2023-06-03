#include "statementCreatorTest.h"
#include "tokeniserTest.h"

int main (int argc, char** args, char** env) {
  tokeniserTest::next ();
  statementCreatorTest::createLabelOrConstantStatement ();
  statementCreatorTest::createInstructionWithOneOperand ();
  return 0;
}
