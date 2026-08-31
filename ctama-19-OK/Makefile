CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lSDL2 -lm

all: game

game: main.c  mysdl2.h canvas.h sweetie-16.h font.h
	$(CC) main.c -o game $(CFLAGS) $(LDFLAGS)

run: all
	./game

clean:
	rm -f game
