#include "VideoMemoryTest.h"

int main (int argc, char** args, char** env) {
  test::readByte_planar_mode0 ();
  test::readByte_planar_mode1 ();
  test::readByte_oddEven ();
  test::readByte_chain4 ();
  test::readByte_memoryMap ();
  test::writeByte_planar_mode0 ();
  test::writeByte_planar_mode1 ();
  test::writeByte_planar_mode2 ();
  test::writeByte_planar_mode3 ();
  test::writeByte_oddEven ();
  test::writeByte_chain4 ();
  test::writeByte_memoryMap ();
  test::getPixels_singleShift ();
  test::getPixels_interleavedShift ();
  test::getPixels_256Shift ();
  test::getPixels_singleShift_planar_cgaAddressing ();
  test::getPixels_interleavedShift_oddEven_cgaAddressing ();
  test::getPixels_256Shift_chain4_cgaAddressing ();
  return 0;
}
