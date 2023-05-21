include ../../common.mk

VPATH=../src

objs=\
  Token.o\
  TokenData.o\
  Tokeniser.o

../asm.elf: $(objs)
	$(CXX) $(LDFLAGS) -o $@ $^
