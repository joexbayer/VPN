#ifndef CRYPTO_H
#define CRYPTO_H value

#include "common.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define KEY_LENGTH 2048
#define PUB_EXP 3


struct crypto_instance
{
	BIGNUM *bne;
	RSA *keypair;

	BIO *pri;
	BIO *pub;

	/* Created using Malloc*/
	char* pri_key;
    char* pub_key;
};

struct crypto_message
{
	char* buffer;
	uint32_t size;
}

struct crypto_instance* crypto_init();
void free_crypto_instance(struct crypto_instance* instance);
struct crypto_message* vpn_encrypt(uint8_t* cleartext, uint32_t size, RSA *myRSA);
struct crypto_message* vpn_decrypt(struct crypto_instance* instance, uint8_t* ciphertext, uint32_t size);


#endif