include ../../common.mk
include ../common_objs.mk

VPATH=../src:../src/exception:../src/expressions

#objs:=$(objs) asm.o

../asm.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
