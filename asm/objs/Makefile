include ../../common.mk
include ../common_objs.mk

VPATH=../src:../src/exception:../src/expressions

objs:=$(objs) asmc.o Config.o

../asmc: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
