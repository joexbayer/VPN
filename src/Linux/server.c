#include "server.h"
#include "vpn_registry.h"
#include "vpn_config.h"

pthread_t tid[2];
static int udp_socket;
static int tun_fd;

#define DEBUG 0

pthread_mutex_t lock;

struct vpn_registry* registry;

void stop_server()
{
    printf("\nStopped.\n");
    free_vpn_registry(registry);
    pthread_mutex_destroy(&lock);
    close(udp_socket);
    exit(EXIT_SUCCESS);

}

/**
 * thread_socket2tun - Fowards packets from socket to tun
 * @arg: potential arguments
 *
 * Fowards all packets received from udp socket to tun interface.
 * 
 * 
 * returns void
 */
void* thread_socket2tun()
{
    char* buffer[2555] = {0};
    struct sockaddr_in client_addr;
    int client_struct_length = sizeof(client_addr);

    while(1)
    {
        int rc = recvfrom(udp_socket, buffer, 2555, 0, (struct sockaddr*)&client_addr,(socklen_t*) &client_struct_length);
        if(rc <= 0)
        {
            continue;
        }

        struct ip_hdr* hdr = (struct ip_hdr*) buffer;
        hdr->saddr = ntohl(hdr->saddr);

        /* look for connection in registry. */
        pthread_mutex_lock(&lock);
        int out_ip = get_vpn_connection_addr(registry, client_addr.sin_addr.s_addr);
        if(out_ip == 0)
        {
            out_ip = register_connection(registry, hdr->saddr, client_addr);
        }
        pthread_mutex_unlock(&lock);

        if(DEBUG)
            printf("recv: %d bytes from virutal ip %d, real ip %d, subnet ip: %d\n", rc, hdr->saddr, client_addr.sin_addr.s_addr, out_ip);

        /* Replace source with given out ip address  */
        hdr->saddr = out_ip;
        hdr->saddr = htonl(hdr->saddr);

        rc = write(tun_fd, buffer, rc);
    }
}

/**
 * thread_tun2socket - Fowards packets from tun to socket
 * @arg: potential arguments
 *
 * Fowards all packets received from tun interface to udp socket.
 * 
 * returns void
 */
void* thread_tun2socket()
{
    char* buffer[2555] = {0};
    struct sockaddr_in client_addr;
    int client_struct_length = sizeof(client_addr);
    while(1)
    {
        int rc = read(tun_fd, buffer, 2555);
        if(rc <= 0)
        {
            continue;
        }

        struct ip_hdr* hdr = (struct ip_hdr*) buffer;
        hdr->daddr = ntohl(hdr->daddr);

        /* Look for connection based on virtual ip inn */
        pthread_mutex_lock(&lock);
        struct vpn_connection* conn = get_vpn_connection_ip(registry, hdr->daddr);
        if(conn == NULL)
        {
            pthread_mutex_unlock(&lock);
            continue;
        }
        pthread_mutex_unlock(&lock);

        /* Replace destination with user chosen ip */
        hdr->daddr = conn->vip_in;
        hdr->daddr = htonl(hdr->daddr);
        
        if(DEBUG)
            printf("sending %d bytes to client real ip %d with virtual ip %d\n", rc, conn->connection->sin_addr.s_addr, hdr->daddr);

        rc = sendto(udp_socket, buffer, rc, 0, (struct sockaddr*)conn->connection, client_struct_length);
    }
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, stop_server);

    /* Create a new VPN registry */
    registry = create_registry((uint8_t*) "10.0.0.1/24");

    struct sockaddr_in server;
    udp_socket = create_udp_socket(&server);

    tun_fd = create_tun_interface();

    int conf = configure_ip_forwarding("10.0.0.1/24");
    if(conf <= 0)
    {
        printf("[ERROR] Could not configure iptables!\n");
        exit(EXIT_FAILURE);
    }


    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


    /* Create thread for incomming client packets */
    int err;
    err = pthread_create(&(tid[0]), NULL, &thread_socket2tun, NULL);
    if(err != 0){
        printf("\ncan't create thread :[%s]", strerror(err));
    }


    /* Create thread for incomming tun packets */
    err = pthread_create(&(tid[1]), NULL, &thread_tun2socket, NULL);
    if(err != 0){
        printf("\ncan't create thread :[%s]", strerror(err));
    }

    while(1)
    {
        sleep(3);
        printf("\rConnected Users: %d", registry->size);
        fflush(stdout);

        pthread_mutex_lock(&lock);
        registry_check_timeout(registry);
        pthread_mutex_unlock(&lock);
    }

    /* code */
    return 0;
}
