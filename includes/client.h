#ifndef CLIENT_H
#define CLIENT_H value

#include "common.h"
#include "vpn_config.h"
#include "crypto.h"


/**
 * vpn_connection - VPN connection for a client
 * @virtual_ip: the virtual IP used for this connection
 * @data_sent: amount of data sent
 * @data_recv: amount of data received.
 * @server_addr: sockaddr_in for server
 * @udp_socket: socket fd for udp.
 * @tun_fd: fd for tun device.
 * @myRSA: Public key for server.
 * @bufio: buf for public key
 *
 * Contains all information for current connection to server.
 * 
 * 
 * returns void
 */
struct vpn_connection
{
	uint32_t virtual_ip;
	uint32_t data_sent;
	uint32_t data_recv;

	struct sockaddr_in server_addr;
	int udp_socket;
	int tun_fd;

	/* Crypto */
	RSA *myRSA;
	BIO *bufio;

};


int start_vpn_client(const char* route, const char* server_ip);

#endif