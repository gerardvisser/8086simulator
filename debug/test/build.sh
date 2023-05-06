#!/bin/bash

mkdir -p objs

#c++ -std=c++17 -c -O2 -I../../8086/include -o objs/videoModes.o videoModes.cpp
c++ -std=c++17 -c -O2 -I../../8086/include -o objs/main.o main.cpp

c++ -std=c++17 -s -Llib -o main.elf objs/*.o -pthread -l8086 -lSDL2
