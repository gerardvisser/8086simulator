#include "VideoMemoryTest.h"

int main (int argc, char** args, char** env) {
  test::readByte_planar_mode0 ();
  test::readByte_planar_mode1 ();
  test::readByte_memoryMap ();
  test::writeByte_planar_mode0 ();
  test::writeByte_planar_mode1 ();
  test::writeByte_planar_mode2 ();
  test::writeByte_planar_mode3 ();
  return 0;
}
