CC=gcc

all : server client

clean:
	rm -rf server client

server: server.c libmemory.so
	gcc -L`pwd`/memory/bin -Wl,-rpath=`pwd`/memory/bin -Wall -o ./server server.c -lmemory -pthread -lrt

client: client.c
	gcc -o client client.c -lrt

libmemory.so: memory.o
	gcc -shared -o memory/bin/libmemory.so memory/bin/memory.o -pthread

memory.o: memory/memory.c
	gcc -c -Wall -Werror -fpic memory/memory.c -o memory/bin/memory.o -pthread
