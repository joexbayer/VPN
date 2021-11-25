#include "server.h"
#include "vpn_registry.h"
#include "vpn_config.h"

int main(int argc, char const *argv[])
{

	/* Create a new VPN registry */
	struct vpn_registry* registry = create_registry("ip");

	/* code */
	return 0;
}