#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
//#include <openssl/err.h>

//gcc rsa_c.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o rsa_c

int main(int argc, char *argv[])
{
	int sockfd = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr;

	if(argc != 2)
	{
		printf("\n Usage: %s <ip of server> \n",argv[0]);
		return 1;
	}

	memset(recvBuff, '0',sizeof(recvBuff));

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(12345);

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	}


	/* Encryption starts here */
	int n = read(sockfd, recvBuff, sizeof(recvBuff)-1);

	const char *p = recvBuff;
	BIO *bufio = BIO_new_mem_buf((void*)p, n);
	RSA *myRSA = PEM_read_bio_RSAPublicKey(bufio, 0, 0, 0);

	char* test = "Joebayer test";

    char* encrypt = malloc(RSA_size(myRSA));
    int encrypt_len = RSA_public_encrypt(strlen(test), (unsigned char*)test, (unsigned char*)encrypt, myRSA, RSA_PKCS1_OAEP_PADDING);

    int rc = send(sockfd , encrypt ,encrypt_len, 0 );
	if(rc < 0)
	{
		printf("\n send error \n");
	}

	return 0;
}