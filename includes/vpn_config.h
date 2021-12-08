#ifndef VPN_CONFIG_H
#define VPN_CONFIG_H value

#include "common.h"

#define VPN_PORT 2000

int create_tun_interface(char* virtual_subnet);
int create_udp_socket(struct sockaddr_in* server_addr, uint8_t* server_ip);
int restore_gateway();
int configure_route(uint8_t* route, uint8_t* server_ip);
int configure_ip_forwarding(char* virtual_subnet);



#endif