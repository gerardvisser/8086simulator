#include <cstdio>
#include "VideoMemoryTest.h"

int main (int argc, char** args, char** env) {
  test::readByte_planar_mode0 ();
  test::readByte_planar_mode1 ();
  return 0;
}
