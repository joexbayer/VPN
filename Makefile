VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra -O2

all: compile run

compile: 
	gcc src/Linux/*.c $(CFLAGS) -o build/server
	gcc src/MacOS/*.c $(CFLAGS) -o build/client

run: compile
	./build/client