include ../../common.mk
include ../common_objs.mk

CXXFLAGS:=$(CXXFLAGS) -I../include

VPATH=../src/8086:../src/8086/exception:../src/8086/sdl

objs:=$(objs)\
  IOTask.o\
  Machine.o\
  SdlRenderer.o\
  SdlWindow.o

../../lib/lib8086.a: $(objs)
	$(AR) $(ARFLAGS) $@ $^
