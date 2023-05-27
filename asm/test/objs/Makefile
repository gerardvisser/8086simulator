include ../../../common.mk
include ../../common_objs.mk

VPATH=..:../../src

objs:=$(objs)\
  test.o\
  tokeniserTest.o

../test.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
