.PHONY: all clean

CC = g++
CFLAGS = -Wall -Werror -Wextra -g
SMFL_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

all: minesweeper

clean:
	rm -f minesweeper

# Regel zum Erstellen des Programms
minesweeper: main.cpp logic.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(SMFL_FLAGS)




#	rm -f main.o
#	rm -f logic.o
# Regel zum Erstellen der Objektdateien
#%.o: %.cpp
#	$(CC) $(CFLAGS) -c $^ -o $@