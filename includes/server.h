#ifndef SERVER_H
#define SERVER_H value

#include "common.h"
#include "vpn_registry.h"
#include "vpn_config.h"
#include "crypto.h"

#define MAX_CONNECTIONS 255

/* VPN connection logic */
int handle_incomming_packet(uint8_t* buffer);
int handle_outgoing_packet(uint8_t* buffer, struct sockaddr_in connection_in);

void start_server(const char* network);

#endif