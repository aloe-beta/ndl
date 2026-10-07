CC = cc
CFLAGS = -Wall -Wextra -Iinclude
SRC = $(wildcard src/*.c)
TARGET = bin/ndl

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf bin
