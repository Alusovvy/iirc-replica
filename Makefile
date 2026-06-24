
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c2x
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = server

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
