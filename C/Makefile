CC=gcc
OPTIONS=--std=gnu11 -g

all:
	$(CC) source/main.c -c -o obj/main.o $(OPTIONS)
	$(CC) source/LinkedStack.c -c -o obj/LinkedStack.o $(OPTIONS)
	$(CC) obj/*.o -o out/main

do:
	@out/main
