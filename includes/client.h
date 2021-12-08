#ifndef CLIENT_H
#define CLIENT_H value

#include "common.h"
#include "vpn_config.h"

struct vpn_connection
{

	uint32_t virtual_ip;
	uint32_t data_sent;
	uint32_t data_recv;
	struct sockaddr_in server_addr;
	int udp_socket;
	int tun_fd;

};


int start_vpn_client(const char* route, const char* server_ip);

#endif