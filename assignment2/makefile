# This is the makefile for ECE1387 Assignment 2 Analytical Placer
# Developer: 	Louis Chen (1000303502)
# Date:		Nov 2, 2018

all:
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o main.o main.cpp
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Placer.o Placer.cpp
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Block.o Block.cpp
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Net.o Net.cpp
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Display.o Display.cpp
	g++ -I./SuiteSparse/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o graphics.o graphics.cpp
	g++ -L./SuiteSparse/lib -o Placer-LouisChen main.o Placer.o Block.o Net.o Display.o graphics.o -lX11 -lumfpack

clean:
	rm -f main.o Placer.o Block.o Net.o Display.o graphics.o Placer-LouisChen
