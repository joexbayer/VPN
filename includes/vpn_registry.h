#ifndef VPN_REGISTRY_H
#define VPN_REGISTRY_H value

#include "../../includes/common.h"
#include "server.h"

/**
 * ConnectionState enum.
 * Used to differentiate between the different states a connection be can in.
 * Especially important for the crypto handshake.
 */
enum ConnectionState
{
    CONNECTED,
    DISCONNECTED,
    ALIVE
};

/** struct vpn_connection
 * Used to identify a client connected to the VPN.
 * 
 * @vip_out: virtual IP that is used to identify the client on the server virtual network.
 * @vip_in: virtual IP that the client specified when connection.
 * @connection: sockaddr_in that contains clients IP and port.
 * @state: connection state enum.
 * @ts: timestamp, used to check if connection is dead.
 * @data_recv: stores how much data client has received
 * @data_sent: stores how much data client has sent.
 * @key: key used for AES encryption.
 * 
 * Struct is needed to be able to sperate incomming traffic based on client.
 */
struct vpn_connection
{

    /* Config */
	uint32_t vip_out;
	uint32_t vip_in;

    /* Connection */
	struct sockaddr_in* connection;
    enum ConnectionState state;

    /* Health */
    struct timeval ts;

    /* Stats */
    uint32_t data_recv;
    uint32_t data_sent;

    /* Crypto */
	uint8_t* key;

}__attribute__((packed));


/** struct vpn_registry
 * A single registry, containing a list of vpn connections
 * 
 * @vpn_connection_registry: A list of vpn connections size of MAX_CONNECTIONS.
 * @vpn_ip: virtual IP range that the registry assigned.
 * @vpn_ip_raw: the uint32 version of the vpn_ip.
 * 
 * @udp_socket: udp socket to receive data from client.
 * @tun_fd: virtual tun device
 * 
 * @data_in: total data received.
 * @data_out: total data sent
 * 
 * @size: total amount of users.
 * 
 * Struct is needed to correlate connections to an ip range.
 */
struct vpn_registry
{

	struct vpn_connection** vpn_connection_registry;
    
	uint8_t* vpn_ip;
	uint32_t vpn_ip_raw;
    uint32_t hosts;

    int udp_socket;
    int tun_fd;

    uint64_t data_in;
    uint64_t data_out;

    uint8_t size;

}__attribute__((packed));

struct vpn_registry* create_registry(uint8_t* ip);

int free_vpn_registry(struct vpn_registry* reg);
struct vpn_connection* register_connection(struct vpn_registry* registry, uint32_t client_virtual_ip, struct sockaddr_in new_connection);
struct vpn_connection* get_vpn_connection_addr(struct vpn_registry* registry, int addr);
struct vpn_connection* get_vpn_connection_ip(struct vpn_registry* registry, int in_ip);

int registry_check_timeout(struct vpn_registry* registry);

#endif