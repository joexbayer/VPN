#include "vpn_registry.h"
#include "server.h"

/**
 * create_registry - Creates a vpn registry, bound to a ip.
 * @ip: ip to assign registry to.
 *
 * Creates a pointer for the registry and assigns ip.
 * Importantly sets all connections to NULL, this is 
 * needed to be able to add, remove and insert new
 * connections.
 * 
 * returns a empty prepared vpn registry.
 */
struct vpn_registry* create_registry(uint8_t* ip)
{
	struct vpn_registry* registry = malloc(sizeof(struct vpn_registry));
	registry->vpn_ip = malloc(strlen((char*) ip)+1);
	strcpy((char*)registry->vpn_ip, (char*)ip);

    struct sockaddr_in sa;
    inet_pton(AF_INET, ip, &sa.sin_addr.s_addr);

	registry->vpn_ip_raw = sa.sin_addr;

    registry->size = 0;

	/* Set connections to NULL */
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		registry->vpn_connection_registry[i] = NULL;
	}

    printf("VPN registry successfully created!\n");

	return registry;
}

/**
 * free_vpn_registry - Frees all memory used by registry.
 * @reg: registry to free.
 *
 * Frees the vpn_ip pointed created by create_registry.
 * And also the registry pointer itself.
 * 
 * returns 1.
 */
inline int free_vpn_registry(struct vpn_registry* reg)
{

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if(reg->vpn_connection_registry[i] == NULL)
        {
            continue;
        }
        free(reg->vpn_connection_registry[i]);
    }

	free(reg->vpn_ip);
	free(reg);

    printf("VPN registry successfully removed.\n");

	return 1;
}

/**
 * register_connection - Registers a new connection
 * @registry: registry to add too
 * @client_virtual_ip: ip specified by client
 * @new_connection: sockdaddr containing real IP off client.
 *
 * Adds a client to a empty registry entry.
 * 
 * returns entry id, MAC_CONNECTIONS if full.
 */
inline int register_connection(struct vpn_registry* registry, uint32_t client_virtual_ip, struct sockaddr_in new_connection)
{

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if(registry->vpn_connection_registry[i] != NULL)
        {
            struct vpn_connection* vpc = malloc(sizeof(struct vpn_connection));
            struct sockaddr_in* conn = malloc(sizeof(struct sockaddr_in));
            memcpy(conn, &new_connection, sizeof(struct sockaddr_in));
            vpc->connection = conn;
            vpc->vip_in = client_virtual_ip;
            vpc->vip_out = registry->vpn_ip_raw + registry->size;

            registry->vpn_connection_registry[i] = vpc;
            registry->size++;

            return i;
        }
    }

    return MAX_CONNECTIONS;

}

/**
 * get_vpn_connection_addr - search for connection
 * @registry: registry to search
 * @addr: addr to search for.
 *
 * Serches for out ip of incomming connection
 * 
 * returns addr or 0
 */
int get_vpn_connection_addr(struct vpn_registry* registry, int addr)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (registry->vpn_connection_registry[i] == NULL)
        {
            continue;
        }

        if(registry->vpn_connection_registry[i]->connection->sin_addr.s_addr == addr)
        {
            return registry->vpn_connection_registry[i]->vip_out;
        }
    }

    return 0;
}

/**
 * get_vpn_connection_ip - search for connection
 * @registry: registry to search
 * @in_ip: in ip to search for.
 *
 * Searches for out ip of incomming connection
 * 
 * returns addr or 0
 */
struct vpn_connection* get_vpn_connection_ip(struct vpn_registry* registry, int in_ip)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (registry->vpn_connection_registry[i] == NULL)
        {
            continue;
        }

        if(registry->vpn_connection_registry[i]->vip_out == in_ip)
        {
            return registry->vpn_connection_registry[i];
        }
    }

    return NULL;
}