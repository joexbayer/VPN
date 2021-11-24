#include "vpn_registry.h"


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
	vpn_registry->vpn_ip = malloc(strlen(ip)+1);
	strcpy(ip, vpn_registry->vpn_ip);

	registry->vpn_ip_raw = 0; // TODO: convert ip to int

	/* Set connections to NULL */
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		vpn_registry->vpn_connection_registry[i] = NULL;
	}

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
int free_vpn_registry(struct vpn_registry* reg)
{
	free(reg->vpn_ip);
	free(reg);

	return 1;
}