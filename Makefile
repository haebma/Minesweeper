.PHONY: all clean

CC = g++
CFLAGS = -Wall -Werror -Wextra -std=c++17
SMFL_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

all: minesweeper

clean:
	rm -f minesweeper

minesweeper: main.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(SMFL_FLAGS)