#include "server.h"
#include "vpn_registry.h"
#include "vpn_config.h"

// NEEDED FOR CONFIGURATION:
/*
sudo sysctl -w net.ipv4.ip_forward=1
sudo ifconfig tun0 172.16.0.1/24 mtu 1400 up
sudo iptables -t nat -A POSTROUTING -s 172.16.0.0/24 ! -d 172.16.0.0/24 -m comment --comment 'vpndemo' -j MASQUERADE
sudo iptables -A FORWARD -s 172.16.0.0/24 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -d 172.16.0.0/24 -j ACCEPT	
*/

int main(int argc, char const *argv[])
{

	/* Create a new VPN registry */
	struct vpn_registry* registry = create_registry("ip");

	/* code */
	return 0;
}