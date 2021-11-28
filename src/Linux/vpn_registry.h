#ifndef VPN_REGISTRY_H
#define VPN_REGISTRY_H value

#include "common.h"
#include "server.h"


/** struct vpn_connection
 * Used to identify a client connected to the VPN.
 * 
 * @vip_out: virtual IP that is used to identify the client on the server virtual network.
 * @vip_in: virtual IP that the client specified when connection.
 * @connection: sockaddr_in that contains clients IP and port.
 * @ts: timestamp, used to check if connection is dead.
 * @key: key used for AES encryption.
 * 
 * Struct is needed to be able to sperate incomming traffic based on client.
 */
struct vpn_connection
{

	uint32_t vip_out;
	uint32_t vip_in;
	struct sockaddr_in* connection;
    struct timeval ts;
	uint8_t* key;

}__attribute__((packed));


/** struct vpn_registry
 * A single registry, containing a list of vpn connections
 * 
 * @vpn_connection_registry: A list of vpn connections size of MAX_CONNECTIONS.
 * @vpn_ip: virtual IP range that the registry assigned.
 * @vpn_ip_raw: the uint32 version of the vpn_ip.
 * 
 * Struct is needed to correlate connections to an ip range.
 */
struct vpn_registry
{

	struct vpn_connection* vpn_connection_registry[MAX_CONNECTIONS];
	uint8_t* vpn_ip;
	uint32_t vpn_ip_raw;
    uint8_t size;

}__attribute__((packed));

struct vpn_registry* create_registry(uint8_t* ip);

int free_vpn_registry(struct vpn_registry* reg);
int register_connection(struct vpn_registry* registry, uint32_t client_virtual_ip, struct sockaddr_in new_connection);

int get_vpn_connection_addr(struct vpn_registry* registry, int addr);
struct vpn_connection* get_vpn_connection_ip(struct vpn_registry* registry, int in_ip);

int registry_check_timeout(struct vpn_registry* registry);

#endif