#ifndef VPN_CONFIG_H
#define VPN_CONFIG_H value

#include "common.h"
#include "server.h"

#define PORT 2000

/* Creation and Configuration */
int create_tun_interface();
int create_udp_socket(struct sockaddr_in* server_addr);

int configure_ip_forwarding(char* virtual_subnet);

#endif