#!/bin/bash

mkdir -p objs

c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/RuntimeException.o ../src/8086/exception/RuntimeException.cpp

c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/Address.o ../src/8086/Address.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/dacData.o ../src/8086/dacData.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/font.o ../src/8086/font.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/Memory.o ../src/8086/Memory.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/VideoCard.o ../src/8086/VideoCard.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/VideoMemory.o ../src/8086/VideoMemory.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/VideoOutputController.o ../src/8086/VideoOutputController.cpp

c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/Colour.o ../src/8086/sdl/Colour.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/Renderer.o ../src/8086/sdl/Renderer.cpp

c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/VideoMemoryTest.o VideoMemoryTest.cpp
c++ -std=c++17 -c -O2 -DTEST_MODE -I../include -o objs/test.o test.cpp

c++ -std=c++17 -s -o test.elf objs/*.o
