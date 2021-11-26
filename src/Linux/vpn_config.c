#include "vpn_config.h"

int configure_ip_forwarding()
{
	/*
	sudo sysctl -w net.ipv4.ip_forward=1
	sudo ifconfig tun0 172.16.0.1/24 mtu 1400 up
	sudo iptables -t nat -A POSTROUTING -s 172.16.0.0/24 ! -d 172.16.0.0/24 -m comment --comment 'vpn' -j MASQUERADE
	sudo iptables -A FORWARD -s 172.16.0.0/24 -m state --state RELATED,ESTABLISHED -j ACCEPT
	sudo iptables -A FORWARD -d 172.16.0.0/24 -j ACCEPT	
	*/

	return 1;
}