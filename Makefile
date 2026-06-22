
CC = gcc
CFLAGS=-Wall -O2 -std=c2x
LDFLAGS=

SRC = src/server.c
OBJ = server.o

all: clean server
	
server:
	$(CC) $(CFLAGS) $(SRC) -o server

clean:
	rm -f *.o server

.PHONY: all clean
