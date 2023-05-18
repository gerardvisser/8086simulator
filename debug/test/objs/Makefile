include ../../../common.mk

CXXFLAGS:=$(CXXFLAGS) -I../../../8086/include
LDFLAGS:=$(LDFLAGS) -L../../../lib
LDLIBS=-pthread -l8086 -lSDL2

VPATH=..

objs:=main.o videoModes.o

../main.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
