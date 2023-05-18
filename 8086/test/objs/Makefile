include ../../../common.mk
include ../../common_objs.mk

CXXFLAGS:=$(CXXFLAGS) -I../../include
CPPFLAGS=-DTEST_MODE

VPATH=..:../../src/8086:../../src/8086/exception:../../src/8086/sdl

objs:=$(objs)\
  test.o\
  videoMemoryTest.o

../test.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
