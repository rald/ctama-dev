CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lSDL2 -lm

TARGET = test_animation

SRC = test_animation.c

all: $(TARGET)

$(TARGET): $(SRC) ../mysdl2.h ../canvas.h ../sweetie-16.h ../font.h
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) -I.. -L..

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)


