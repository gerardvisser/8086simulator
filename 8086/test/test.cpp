#include <cstdio>

#include <8086/Address.h>

void print (Address address) {
  int ad = address;

  printf ("segment=%04X, offset=%04X, address=%06X (%d)\n", address.segment, address.offset, ad, ad);
}

int main (int argc, char** args, char** env) {
  Address address (0xF001, 0xFFEF);
  print (address);
  return 0;
}
