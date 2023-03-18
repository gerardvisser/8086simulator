#!/bin/bash

mkdir -p objs

c++ -std=c++17 -c -I../include -o objs/RuntimeException.o ../src/8086/exception/RuntimeException.cpp

c++ -std=c++17 -c -I../include -o objs/Address.o ../src/8086/Address.cpp
c++ -std=c++17 -c -I../include -o objs/test.o test.cpp

c++ -std=c++17 -o test.elf objs/*.o
