#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <sys/select.h> 
//gcc rsa.c -I/usr/local/opt/openssl@3/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -o rsa

#define KEY_LENGTH 2048
#define PUB_EXP 3

int main(void) {
    char   *encrypt = NULL;    // Encrypted message
    char   *decrypt = NULL;    // Decrypted message
    char   *err;               // Buffer for any error messages

    BIGNUM *bne = BN_new();
    int ret = BN_set_word(bne, PUB_EXP);

    // Generate key pair
    printf("Generating RSA (%d bits) keypair...", KEY_LENGTH);
    fflush(stdout);

    // create key
    RSA *keypair = RSA_new();
    ret = RSA_generate_key_ex(keypair, KEY_LENGTH, bne, NULL);

    // To get the C-string PEM form:
    BIO *pri = BIO_new(BIO_s_mem());
    BIO *pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_RSAPublicKey(pub, keypair);

    // key lengths
    size_t pri_len = BIO_pending(pri);
    size_t pub_len = BIO_pending(pub);

    // char* allocs.
    char* pri_key = malloc(pri_len + 1);
    char* pub_key = malloc(pub_len + 1);

    // read in BIO
    BIO_read(pri, pri_key, pri_len);
    BIO_read(pub, pub_key, pub_len);

    // add 0 terminator.
    pri_key[pri_len] = '\0';
    pub_key[pub_len] = '\0';

    printf("\n%s\n%s\n", pri_key, pub_key);
    printf("done.\n");


    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char *hello = "HTTP/1.1 302 Found\nLocation: http://www.iana.org/domains/example/\n\n";
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
   
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( 12345 );

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }



     while(1){

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("connectioN!\n");
        int rc = send(new_socket , pub_key ,strlen(pub_key), 0 );
        char* buffer2[64000] = {0};
        int rc_recv = recv(new_socket , buffer2 , 64000 , 0);
        if(rc_recv == 0){
            return 1;
        }
        printf("decrypting.\n");

        decrypt = malloc(30000);
        if(RSA_private_decrypt(rc_recv, (unsigned char*)buffer2, (unsigned char*)decrypt,
                               keypair, RSA_PKCS1_OAEP_PADDING) == -1) {
            ERR_load_crypto_strings();
            ERR_error_string(ERR_get_error(), err);
            fprintf(stderr, "Error decrypting message: %s\n", err);
            goto free_stuff;
        }
        printf("%s\n", decrypt);
    }

    free_stuff:
    RSA_free(keypair);
    BIO_free_all(pub);
    BIO_free_all(pri);
    free(pri_key);
    free(pub_key);
    free(encrypt);
    free(decrypt);
    free(err);

    return 0;
}