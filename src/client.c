#include "../includes/client.h"

static pthread_t tid[2];
static struct vpn_connection* current_connection;

static const unsigned char key[] = "01234567890123456789012345678901";

/**
 * stop_client - Signal function
 * @arg: potential arguments
 *
 * Closes all fd's and frees memory.
 * 
 * returns void
 */
void stop_client()
{
	restore_gateway();
	printf("\nStopped.\n");
	close(current_connection->udp_socket);
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
	while(1)
	{
        int rc = read(current_connection->udp_socket, buffer, 2555);
        if(rc <= 0)
        {
        	continue;
        }

        /* Decrypt */
        unsigned char decryptedtext[20000];
        unsigned char* tag = (unsigned char*) buffer;

        int decrypted_len = vpn_aes_decrypt(buffer+16, rc-16, aad, strlen(aad), tag, key, IV, decryptedtext);
        if(decrypted_len < 0)
        {
            /* Verify error */
            printf("Decrypted text failed to verify\n");
            continue;
        }

        current_connection->data_sent += decrypted_len;
        rc = write(current_connection->tun_fd, decryptedtext, decrypted_len);
	}
}

/**
 * thread_tun2socket - Fowards packets from tun to socket
 * @arg: potential arguments
 *
 * Fowards all packets received from tun interface to udp socket.
 * 
 * 
 * returns void
 */
void* thread_tun2socket()
{
	char* buffer[2555] = {0};
	while(1)
	{
        int rc = read(current_connection->tun_fd, buffer, 2555);
        if(rc <= 0)
        {
        	continue;
        }

        /* Encrypt */
        unsigned char ciphertext[20000];
        unsigned char tag[16];
        int cipher_len = vpn_aes_encrypt(buffer, rc, aad, strlen(aad), key, IV, ciphertext, tag);

        unsigned char* encrypt_tag = malloc(cipher_len+16);
        memcpy(encrypt_tag, tag, 16);
        memcpy(encrypt_tag+16, ciphertext, cipher_len);

        current_connection->data_recv += cipher_len;
        rc = sendto(current_connection->udp_socket, encrypt_tag, cipher_len+16, 0, (struct sockaddr*)&(current_connection->server_addr), sizeof(current_connection->server_addr));
        free(encrypt_tag);
	}
}

/**
 * start_threads - Starts the two threads for handling data.
 * @void:
 *
 * Starts two threads, one for sending data received from tun.
 * Other one puts data into tun from socket.
 * 
 * 
 * returns void
 */
static void start_threads()
{
	/* Create thread for incomming client packets */
    int err;
    err = pthread_create(&(tid[0]), NULL, &thread_socket2tun, NULL);
    if(err != 0)
    {
		printf("[ERROR] Could not create socket2tun thread.\n");
		exit(EXIT_FAILURE);
    }

    /* Create thread for incomming tun packets */
	err = pthread_create(&(tid[1]), NULL, &thread_tun2socket, NULL);
    if(err != 0)
    {
		printf("[ERROR] Could not create tun2socket thread.\n");
		exit(EXIT_FAILURE);
    }

}

/** handshake
 * Handshake for exchanging crypto information from server.
 * @void: 
 * 
 * Connects to the server, exchanges public key and sends over 
 * symmetric secret key.
 * 
 *  returns void.
 */
static void handshake()
{

	char* syn = "connect";
	int rc = sendto(current_connection->udp_socket, syn, strlen(syn), 0, (struct sockaddr*)&(current_connection->server_addr), sizeof(current_connection->server_addr));

	char buffer[1024];
    rc = read(current_connection->udp_socket, buffer, sizeof(buffer)-1);
    if(rc <= 0)
    {
    	printf("Could not read public key\n");
    }

    printf("Received public key from server:\n%s\n", buffer);

    const char *p = buffer;
	current_connection->bufio = BIO_new_mem_buf((void*)p, rc);
	current_connection->myRSA = PEM_read_bio_RSAPublicKey(current_connection->bufio, 0, 0, 0);

	struct crypto_message* msg = vpn_rsa_encrypt(key, strlen(key), current_connection->myRSA);

   	printf("%d\n", msg->size);

	rc = sendto(current_connection->udp_socket, msg->buffer, msg->size, 0, (struct sockaddr*)&(current_connection->server_addr), sizeof(current_connection->server_addr));
	rc = read(current_connection->udp_socket, buffer, 100);
   	if(rc <= 0)
    {
    	printf("Could not read ok\n");
    }

    printf("%s", buffer);
    free(msg->buffer);
    free(msg);
}

int start_vpn_client(const char* route, const char* server_ip)
{
	signal(SIGINT, stop_client);

	current_connection = malloc(sizeof(struct vpn_connection));
	OpenSSL_add_all_algorithms();

	/* Create UDP socket. */
	current_connection->udp_socket = create_udp_socket(&(current_connection->server_addr), (uint8_t*) server_ip);
	if(current_connection->udp_socket <= 0)
	{
		printf("[ERROR] Could not create UDP socket.\n");
		exit(EXIT_FAILURE);
	}

	/* Create TUN interface. */
	current_connection->tun_fd = create_tun_interface("10.0.0.1/24");
	if(current_connection->tun_fd <= 0)
	{
		printf("[ERROR] Could not create TUN device.\n");
		exit(EXIT_FAILURE);
	}

	/* Configure all IP routes. */
	int conf = configure_route((uint8_t*) route, (uint8_t*) server_ip);
	if(conf < 0)
	{
		printf("[ERROR] Could not configure ip route.\n");
		exit(EXIT_FAILURE);
	}

	printf("VPN Client is Connecting...\n");
	handshake();
    printf("VPN Client is Connected.\n");

	/* Start socket / TUN threads */
	start_threads();

    while(1)
    {
        sleep(2);
        printf("\rStats - Sent: %d kb/s, Recv: %d kb/s", (current_connection->data_sent/1024)/2, (current_connection->data_recv/1024)/2);
        fflush(stdout);
        current_connection->data_sent = 0;
        current_connection->data_recv = 0;
    }

	return 0;
}

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		printf("Usage: %s <route> <vpn ip>", argv[0]);
		exit(EXIT_FAILURE);
	}

	start_vpn_client(argv[1], argv[2]);
}