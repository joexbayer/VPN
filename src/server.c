#include "../includes/server.h"

#define DEBUG 0

/* Threads */
pthread_t tid[2];
pthread_mutex_t lock;

struct vpn_registry* registry;
struct crypto_instance* crypto;

void stop_server()
{
    printf("\nStopped.\n");
    free_vpn_registry(registry);
    pthread_mutex_destroy(&lock);
    close(registry->udp_socket);
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
        int rc = recvfrom(registry->udp_socket, buffer, 2555, 0, (struct sockaddr*)&client_addr,(socklen_t*) &client_struct_length);
        if(rc <= 0)
        {
            continue;
        }

        struct ip_hdr* hdr = (struct ip_hdr*) buffer;
        hdr->saddr = ntohl(hdr->saddr);

        /* look for connection in registry. */
        pthread_mutex_lock(&lock);
        struct vpn_connection* conn = get_vpn_connection_addr(registry, client_addr.sin_addr.s_addr);
        if(conn == NULL)
        {
            conn = register_connection(registry, hdr->saddr, client_addr);
            if(conn == NULL)
            {
                printf("[warning] Cannot accept more connections!\n");
                pthread_mutex_unlock(&lock);
                continue;
            }
        }
        pthread_mutex_unlock(&lock);


        switch(conn->state)
        {
            case CONNECTED:
                rc = sendto(registry->udp_socket, crypto->pub_key, strlen(crypto->pub_key), 0, (struct sockaddr*)conn->connection, client_struct_length);
                conn->state = REGISTERED;

                //if(DEBUG)
                printf("Sent public key to new client\n");

                break;

            case REGISTERED:
                ;
                printf("Received %d bytes\n", rc);
                struct crypto_message* msg = vpn_decrypt(crypto, buffer, rc);
                if(msg == NULL)
                {
                    printf("Client sent invalid message in REGISTERED state\n");
                    continue;
                }

                /* Allocate memory for key and add 0 terminator */
                conn->key = malloc(msg->size+1);
                memcpy(conn->key, msg->buffer, msg->size);
                conn->key[msg->size+1] = 0;

                conn->state = ALIVE;

                printf("Registered new key for connection: %s\n", conn->key);

                char* ok = "OK";
                rc = sendto(registry->udp_socket, ok, strlen(ok), 0, (struct sockaddr*)conn->connection, client_struct_length);
                break;

            case ALIVE:
                if(DEBUG)
                    printf("recv: %d bytes from virutal ip %d, real ip %d, subnet ip: %d\n", rc, hdr->saddr, client_addr.sin_addr.s_addr, conn->vip_out);

                /* Replace source with given out ip address  */
                hdr->saddr = conn->vip_out;
                hdr->saddr = htonl(hdr->saddr);

                conn->data_sent += rc;
                registry->data_out += rc;
                rc = write(registry->tun_fd, buffer, rc);
                break;
        }
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
        int rc = read(registry->tun_fd, buffer, 2555);
        if(rc <= 0)
        {
            continue;
        }

        struct ip_hdr* hdr = (struct ip_hdr*) buffer;
        hdr->daddr = ntohl(hdr->daddr);

        /* Look for connection based on virtual ip inn */
        pthread_mutex_lock(&lock);
        struct vpn_connection* conn = get_vpn_connection_ip(registry, hdr->daddr);
        pthread_mutex_unlock(&lock);
        if(conn == NULL)
        {
            continue;
        }

        /* Replace destination with user chosen ip */
        hdr->daddr = conn->vip_in;
        hdr->daddr = htonl(hdr->daddr);
        
        if(DEBUG)
            printf("sending %d bytes to client real ip %d with virtual ip %d\n", rc, conn->connection->sin_addr.s_addr, hdr->daddr);

        conn->data_recv += rc;
        registry->data_in += rc;
        rc = sendto(registry->udp_socket, buffer, rc, 0, (struct sockaddr*)conn->connection, client_struct_length);
    }
}

/**
 * start_server - Main event loop for server
 * @network: nework ip with cidr
 *
 * Creates and configures everything for VPN
 * Creates threads and handles main loop
 * 
 * returns void
 */
void start_server(const char* network)
{
    signal(SIGINT, stop_server);

    /* Create a new VPN registry */
    registry = create_registry((uint8_t*) network);

    /* Create Crypto instance */
    crypto = crypto_init();

    struct sockaddr_in server;
    registry->udp_socket = create_udp_socket(&server, "0");
    registry->tun_fd = create_tun_interface(network);

    int conf = configure_ip_forwarding(network);
    if(conf < 0)
    {
        printf("[ERROR] Could not configure iptables!\n");
        exit(EXIT_FAILURE);
    }

    /* init lock for threads */
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(EXIT_FAILURE);
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

        printf("\rConnected Users: %d, Sending: %d kb/s, Receving: %d kb/s", registry->size, (registry->data_in/1024)/3, (registry->data_out/1024)/3);
        fflush(stdout);

        pthread_mutex_lock(&lock);
        registry_check_timeout(registry);
        pthread_mutex_unlock(&lock);

        registry->data_in = 0;
        registry->data_out = 0;
    }
}

int main()
{
    start_server("10.0.0.1/24");
    /* code */
    return 0;
}
