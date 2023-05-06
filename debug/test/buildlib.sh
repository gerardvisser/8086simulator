#!/bin/bash

mkdir -p lib/objs

rm -f lib/lib8086.a

#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/RuntimeException.o ../../8086/src/8086/exception/RuntimeException.cpp

#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/Address.o ../../8086/src/8086/Address.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/dacData.o ../../8086/src/8086/dacData.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/font.o ../../8086/src/8086/font.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/Machine.o ../../8086/src/8086/Machine.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/Memory.o ../../8086/src/8086/Memory.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/VideoCard.o ../../8086/src/8086/VideoCard.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/VideoMemory.o ../../8086/src/8086/VideoMemory.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/VideoOutputController.o ../../8086/src/8086/VideoOutputController.cpp

#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/Colour.o ../../8086/src/8086/sdl/Colour.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/IOTask.o ../../8086/src/8086/sdl/IOTask.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/Renderer.o ../../8086/src/8086/sdl/Renderer.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/SdlRenderer.o ../../8086/src/8086/sdl/SdlRenderer.cpp
#c++ -std=c++17 -c -O2 -I../../8086/include -o lib/objs/SdlWindow.o ../../8086/src/8086/sdl/SdlWindow.cpp

ar rcs lib8086.a lib/objs/*.o

mv lib8086.a lib

#rm -rf lib/objs
