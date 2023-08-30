include ../../../common.mk
include ../../common_objs.mk

CPPFLAGS=-DTEST_MODE

VPATH=..:../../src:../../src/exception:../../src/expressions

objs:=$(objs)\
  statementCreatorTest.o\
  StatementData.o\
  test.o\
  testUtils.o\
  tokeniserTest.o

../test.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
