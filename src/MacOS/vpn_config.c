#include "vpn_config.h"

static char* gateway;
/**
 * restore_gateway - Restores saved gateway
 * @void:
 *
 * Restores the last saved gateway when program exits.
 * 
 * returns 1.
 */
int restore_gateway()
{
	char cmd [1000] = {0x0};
    sprintf(cmd,"route add default %s", gateway);
    system("route delete default");
    system(cmd);

    return 1;
}

/**
 * save_current_gateway - Saves the current gateway.
 * @void:
 *
 * Saves the current gateway address to be able
 * to restore it when program exists.
 * 
 * returns 1.
 */
static int save_current_gateway()
{
	char cmd [1000] = {0x0};
    sprintf(cmd,"route -n get default | grep gateway | cut -d ':' -f 2 | awk '{$1=$1};1'"); // iOS specific!
    FILE* fp = popen(cmd, "r");
    char line[256]={0x0};

    if(fgets(line, sizeof(line), fp) != NULL){
        gateway = malloc(strlen(line)+1);
        strcpy(gateway, line);
    }
    pclose(fp);

    return 1;
}

/**
 * configure_route - configures IP route.
 * @route: route to add
 * @server_ip: ip of vpn server.
 *
 * Configures IP routes to forward traffic
 * to tun device, but allow traffic out to 
 * vpn server.
 * 
 * returns 1
 */
int configure_route(uint8_t* route, uint8_t* server_ip)
{
	int save = save_current_gateway();
	if(save <= 0)
	{
		printf("[ERROR] Could not save current gateway!\n");
		exit(EXIT_FAILURE);
	}
	/* Delete default route and add new. */
	system("route delete default");

	char cmd [1000] = {0x0};
    sprintf(cmd,"route add %s 10.0.0.255", route);
    system(cmd);

	/* Add rule to allow traffic to vpn server */
    sprintf(cmd,"route add %s %s", server_ip, gateway);
    system(cmd);

    return 1;
}

/**
 * create_tun_interface - Opens a tun device.
 * @void:
 *
 * Creates and opens tun0 interface device.
 * Also configurates the tun0 device to point
 * to correct virtual subnet.
 * 
 * returns fd of device or -1 on error.
 */
int create_tun_interface()
{
	int fd;
    if( (fd = open("/dev/tun0", O_RDWR)) < 0 ) {
        perror("Cannot open tun0 dev\n");
        exit(1);
    }
    system("ifconfig tun0 inet 10.0.0.2 10.0.0.255 up");

    return fd;   
}

/**
 * create_udp_socket - Opens a tun device.
 * @server_addr: sockaddr_in to configure
 * @server_ip: IP that socket should connect too.
 *
 * Creates UDP socket and configures sockaddr_in
 * to point to correct server ip and sin_family.
 * 
 * returns fd of device or -1 on error.
 */
int create_udp_socket(struct sockaddr_in* server_addr, uint8_t* server_ip)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0){
          perror("sock:");
          exit(1);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(VPN_PORT);
    server_addr->sin_addr.s_addr = inet_addr((char*) server_ip);

    return sockfd;

}
