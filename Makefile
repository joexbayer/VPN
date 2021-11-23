VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra

all: compile run

compile: 
	gcc *.c $(CFLAGS) -o build/server

run: compile
	rm -r ./build/server.dSYM && sudo ./build/server


tunpy: tun.py
	sudo python3 tun.py && sudo ifconfig tap12 inet 10.0.0.1 10.0.0.2 up