include ../../../common.mk
include ../../common_objs.mk

VPATH=..:../../src:../../src/exception:../../src/expressions

objs:=$(objs)\
  statementCreatorTest.o\
  test.o\
  testUtils.o\
  tokeniserTest.o

../test.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
