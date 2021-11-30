VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra -O2

all: build-mac
	
build-mac: src/MacOS/*.c
	gcc src/MacOS/*.c -pthread $(CFLAGS) -o client.out

build-linux: src/Linux/*.c
	gcc src/Linux/*.c -pthread $(CFLAGS) -o server.out

client: build-mac
	sudo ./client.out default 3.68.33.50

server: build-linux
	sudo ./server.out

clean: server.out client.out
	rm server.out client.out 