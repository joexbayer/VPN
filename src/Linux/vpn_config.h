#ifndef VPN_CONFIG
#define VPN_CONFIG value

#include "common.h"
#include "server.h"

/* Creation and Configuration */
int create_tun_interface();
int create_udp_socket(uint16_t port);
int configure_tun_device(uint8_t* virtual_ip);
int configure_ip_forwarding();

/* Optional */
int configure_whitelist(uint8_t* filename);


#endif