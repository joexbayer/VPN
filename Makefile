VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra -O2

COMMON = src/vpn_config.c src/vpn_registry.c lib/crypto.c
CRYPTO_FLAGS = -lm -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto

all: build-client

rsa: build-rsa-example
	
build-client: src/client.c $(COMMON)
	gcc src/client.c $(COMMON) -pthread $(CFLAGS) $(CRYPTO_FLAGS) -o client.out

build-server: src/server.c $(COMMON)
	gcc src/server.c $(COMMON) $(CFLAGS) $(CRYPTO_FLAGS) -pthread -o server.out

build-rsa-example:
	gcc lib/example/rsa.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o ./build/rsa_server.out
	gcc lib/example/rsa_c.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o ./build/rsa_client.out

client: build-client
	sudo ./client.out default 18.192.21.126

server: build-server
	sudo ./server.out

clean: server.out client.out
	rm server.out client.out 