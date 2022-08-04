CC=gcc

all : main

clean:
	rm -rf main bin/*

main: main.c libmemory.so
	gcc -L`pwd`/bin -Wl,-rpath=`pwd`/bin -Wall -o main main.c -lmemory -pthread

libmemory.so: memory.o
	gcc -shared -o bin/libmemory.so bin/memory.o -pthread

memory.o: memory.c
	gcc -c -Wall -Werror -fpic memory.c -o bin/memory.o -pthread
