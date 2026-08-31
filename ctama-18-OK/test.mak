CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lSDL2 -lm

all: test

test: test.c mysdl2.h canvas.h sweetie-16.h font.h
	$(CC) test.c -o test $(CFLAGS) $(LDFLAGS)

run: all
	./test

clean:
	rm -f test

