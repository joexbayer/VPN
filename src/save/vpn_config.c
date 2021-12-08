#include "vpn_config.h"


/**
 * create_tun_interface - Creates a tun device
 * @void:
 *
 * Openes a TUN device and configures it.
 * Adds a device name and uses ioctl
 * 
 * returns tun fd
 */
int create_tun_interface()
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) == -1 ) {
           perror("open /dev/net/tun");
           exit(1);
    }

    char* devname = "tun0";
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ); // devname = "tun0" or "tun1", etc

    /* ioctl will use ifr.if_name as the name of TUN
         * interface to open: "tun0", etc. */
    if ( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) == -1 ) {
        perror("ioctl TUNSETIFF");
        close(fd);
        exit(1);
    }

    return fd;
}





/**
 * configure_ip_forwarding - Configures IP
 * @virtual_subnet: ip of the virtual subnut
 *
 * Uses ifconfig to assign IP to tun device.
 * Uses iptables to enable ip forwarding
 * and creates MASQUERADE for outgoing traffic.
 * 
 * returns value of system command.
 */
int configure_ip_forwarding(char* virtual_subnet)
{

    char cmd [1000] = {0x0};
    sprintf(cmd,"ifconfig tun0 %s up", virtual_subnet);
    int sys = system(cmd);
    sys = system("sysctl -w net.ipv4.ip_forward=1");

    sprintf(cmd,"iptables -t nat -A POSTROUTING -s %s ! -d %s -m comment --comment 'vpn' -j MASQUERADE", virtual_subnet, virtual_subnet);
    sys = system(cmd);

    sprintf(cmd,"iptables -A FORWARD -s %s -m state --state RELATED,ESTABLISHED -j ACCEPT", virtual_subnet);
    sys = system(cmd);

    sprintf(cmd,"iptables -A FORWARD -d %s -j ACCEPT", virtual_subnet);
    sys = system(cmd);

    return sys;
}

/**
 * create_udp_socket - Creates UDP socket
 * @server_addr: server address to configure
 *
 * Creates new socket fd with sockeraddr_in
 * Sets socket options and binds socket
 * 
 * returns socket fd.
 */
int create_udp_socket(struct sockaddr_in* server_addr)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0){
          perror("sock:");
          exit(1);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(VPN_PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    return sockfd;
}
