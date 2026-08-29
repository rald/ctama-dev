# Makefile for mysdl2 showcase

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lSDL2 -lm

TARGET = game
SRC = main.c

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lm
endif

all: $(TARGET)

$(TARGET): $(SRC) mysdl2.h
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
