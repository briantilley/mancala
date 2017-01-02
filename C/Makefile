CC=gcc
OPTIONS=--std=gnu11 -Wall -Werror -Wextra -Wfatal-errors -g -O3

all: main
	$(CC) obj/*.o -o out/main

main:
	$(CC) source/main.c -c -o obj/main.o $(OPTIONS)

do:
	@out/main
