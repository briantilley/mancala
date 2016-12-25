CC=gcc
OPTIONS=--std=gnu11 -Wall -Werror -Wextra -Wfatal-errors -g -O3

all: main LinkedStack MemoryBuffer
	$(CC) obj/*.o -o out/main

main:
	$(CC) source/main.c -c -o obj/main.o $(OPTIONS)

LinkedStack:
	$(CC) source/LinkedStack.c -c -o obj/LinkedStack.o $(OPTIONS)

MemoryBuffer:
	$(CC) source/MemoryBuffer.c -c -o obj/MemoryBuffer.o $(OPTIONS)

do:
	@out/main
