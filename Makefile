VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra -O2

SERVER_FILES = src/server.c src/vpn_config.c src/vpn_registry.c lib/crypto.c
CRYPTO_FLAGS = -lm -o -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto

all: build-client

rsa: build-rsa-example
	
build-client: src/client.c src/vpn_config.c src/vpn_registry.c
	gcc src/client.c src/vpn_config.c src/vpn_registry.c -pthread $(CFLAGS) -lm -o client.out

build-server: $(SERVER_FILES)
	gcc $(SERVER_FILES) $(CFLAGS) $(CRYPTO_FLAGS) -pthread -o server.out

build-rsa-example:
	gcc lib/example/rsa.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o ./build/rsa_server.out
	gcc lib/example/rsa_c.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o ./build/rsa_client.out

client: build-client
	sudo ./client.out default 18.192.21.126

server: build-server
	sudo ./server.out

clean: server.out client.out
	rm server.out client.out 