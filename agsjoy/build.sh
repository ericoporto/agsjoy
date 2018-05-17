#!/bin/bash
#tested in ubuntu 16.04

g++ -std=c++0x -shared -o libagsjoy.so -fPIC  agsjoy.cpp `sdl2-config --cflags --libs`
#g++ -lSDL2 -shared -o libagsjoy32.so -fPIC -m32 agsjoy.cpp 
