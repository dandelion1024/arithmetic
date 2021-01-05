CC = gcc

all: arithmetic
arithmetic: arithmetic.c
	$(CC) $^ -o $@ -g

clean:
	rm arithmetic