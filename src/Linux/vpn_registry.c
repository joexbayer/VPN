#include "vpn_registry.h"
#include "server.h"


#define DEBUG 0

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

    /* Allocate IP and remove CIDR */
    registry->vpn_ip = malloc(strlen((char*) ip)-2);
    memcpy((char*)registry->vpn_ip, (char*)ip, strlen((char*)ip)-3);
    registry->vpn_ip[strlen((char*) ip)-2] = 0;

    struct sockaddr_in sa;
    inet_pton(AF_INET, (char *)registry->vpn_ip, &sa.sin_addr);

    if(DEBUG)
        printf("converted ip %s to %d\n", registry->vpn_ip, sa.sin_addr.s_addr);

    registry->vpn_ip_raw = sa.sin_addr.s_addr;
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
        free(reg->vpn_connection_registry[i]->connection);
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
 * returns connection
 */
inline struct vpn_connection* register_connection(struct vpn_registry* registry, uint32_t client_virtual_ip, struct sockaddr_in new_connection)
{

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if(registry->vpn_connection_registry[i] == NULL)
        {
            struct vpn_connection* vpc = malloc(sizeof(struct vpn_connection));
            struct sockaddr_in* conn = malloc(sizeof(struct sockaddr_in));
            memcpy(conn, &new_connection, sizeof(struct sockaddr_in));
            vpc->connection = conn;
            vpc->vip_in = client_virtual_ip;
            vpc->vip_out = htonl(registry->vpn_ip_raw) + (i+1);

            if(DEBUG)
                printf("Assigned new ip: %d, from %d + %d\n",vpc->vip_out,  registry->vpn_ip_raw, registry->size);

            registry->vpn_connection_registry[i] = vpc;
            registry->size++;

            return vpc;
        }
    }

    return NULL;

}

/**
 * get_vpn_connection_addr - search for connection
 * @registry: registry to search
 * @addr: addr to search for.
 *
 * Serches for out ip of incomming connection
 * 
 * returns connection
 */
struct vpn_connection* get_vpn_connection_addr(struct vpn_registry* registry, int addr)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (registry->vpn_connection_registry[i] == NULL)
        {
            continue;
        }

        if(registry->vpn_connection_registry[i]->connection->sin_addr.s_addr == (uint32_t) addr)
        {
            gettimeofday(&(registry->vpn_connection_registry[i]->ts), 0);
            return registry->vpn_connection_registry[i];
        }
    }

    return NULL;
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
    int index = htonl(registry->vpn_ip_raw) - in_ip - 1;
    return registry->vpn_connection_registry[index];
}

/**
 * get_vpn_connection_ip - search for connection
 * @registry: registry to check
 *
 * Look if any connections have timed out
 * 
 * returns 1
 */
int registry_check_timeout(struct vpn_registry* registry)
{
    struct timeval now;
    gettimeofday(&now, 0);

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (registry->vpn_connection_registry[i] != NULL)
        {
            double dif = (now.tv_sec - registry->vpn_connection_registry[i]->ts.tv_sec) * 1000 + (now.tv_usec - registry->vpn_connection_registry[i]->ts.tv_usec) / 1000;
            if(dif > 3000)
            {
                free(registry->vpn_connection_registry[i]->connection);
                free(registry->vpn_connection_registry[i]);
                registry->vpn_connection_registry[i] = NULL;
                registry->size -= 1;
                continue;
            }

            registry->vpn_connection_registry[i]->data_recv = 0;
            registry->vpn_connection_registry[i]->data_sent = 0;


        }
    }

    return 1;
}