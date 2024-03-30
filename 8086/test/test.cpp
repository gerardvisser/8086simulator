#include "processorTest.h"
#include "registersTest.h"
#include "videoMemoryTest.h"

int main (int argc, char** args, char** env) {
  videoMemoryTest::readByte_planar_mode0 ();
  videoMemoryTest::readByte_planar_mode1 ();
  videoMemoryTest::readByte_oddEven ();
  videoMemoryTest::readByte_chain4 ();
  videoMemoryTest::readByte_memoryMap ();
  videoMemoryTest::writeByte_planar_mode0 ();
  videoMemoryTest::writeByte_planar_mode1 ();
  videoMemoryTest::writeByte_planar_mode2 ();
  videoMemoryTest::writeByte_planar_mode3 ();
  videoMemoryTest::writeByte_oddEven ();
  videoMemoryTest::writeByte_chain4 ();
  videoMemoryTest::writeByte_memoryMap ();
  videoMemoryTest::getPixels_singleShift ();
  videoMemoryTest::getPixels_interleavedShift ();
  videoMemoryTest::getPixels_256Shift ();
  videoMemoryTest::getPixels_singleShift_planar_cgaAddressing ();
  videoMemoryTest::getPixels_interleavedShift_oddEven_cgaAddressing ();
  videoMemoryTest::getPixels_256Shift_chain4_cgaAddressing ();
  videoMemoryTest::getPixels_text8x16 ();
  videoMemoryTest::getPixels_text9x16 ();
  videoMemoryTest::getPixels_text9x16LineGraphicsEnable ();
  videoMemoryTest::getPixels_singleShift_maxScanLine_wideChars ();
  videoMemoryTest::getPixels_interleavedShift_maxScanLine_wideChars ();
  videoMemoryTest::getPixels_256Shift_maxScanLine_wideChars ();
  registersTest::getGen ();
  registersTest::setGen ();
  processorTest::mov ();
  processorTest::pushPop ();
  processorTest::pushfPopf ();
  return 0;
}
