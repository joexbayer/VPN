#ifndef SERVER_H
#define SERVER_H value

#include "common.h"

#define MAX_CONNECTIONS 255

/* Encryption OpenSSL AES MOVE OUT */
int configure_encryption(uint8_t* key);
int encrypt(uint8_t* cleartext, uint8_t* encrypted);
int decrypt(uint8_t* ciphertext, uint8_t* decrypted);

/* VPN connection logic */
int handle_incomming_packet(uint8_t* buffer);
int handle_outgoing_packet(uint8_t* buffer, struct sockaddr_in connection_in);

void start_server(const char* network);

#endif