#ifndef CLIENT_H
#define CLIENT_H value

#include "common.h"
#include "vpn_config.h"

struct vpn_connection
{

	uint32_t virtual_ip;
	uint32_t data_sent;
	uint32_t data_recv;

}__attribute__((packed));


int start_vpn_client(char* route, char* server_ip);

#endif