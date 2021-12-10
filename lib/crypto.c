#include "../includes/crypto.h"

struct crypto_instance* crypto_init()
{

	struct crypto_instance* instance = malloc(sizeof(struct crypto_instance));

    instance->bne = BN_new();
    int ret = BN_set_word(instance->bne, PUB_EXP);

    // Generate key pair
    printf("Generating RSA (%d bits) keypair...\n", KEY_LENGTH);

    // create key
    instance->keypair = RSA_new();
    ret = RSA_generate_key_ex(instance->keypair, KEY_LENGTH, instance->bne, NULL);

    // To get the C-string PEM form:
    instance->pri = BIO_new(BIO_s_mem());
    instance->pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(instance->pri, instance->keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_RSAPublicKey(instance->pub, instance->keypair);

    // key lengths
    size_t pri_len = BIO_pending(instance->pri);
    size_t pub_len = BIO_pending(instance->pub);

    // char* allocs.
    instance->pri_key = malloc(pri_len + 1);
    instance->pub_key = malloc(pub_len + 1);

    // read in BIO
    BIO_read(instance->pri, instance->pri_key, pri_len);
    BIO_read(instance->pub, instance->pub_key, pub_len);

    // add 0 terminator.
    instance->pri_key[pri_len] = '\0';
    instance->pub_key[pub_len] = '\0';

    printf("\n%s\n", instance->pub_key);
    printf("Created RSA keypair.\n");

    return instance;
};

void free_crypto_instance(struct crypto_instance* instance)
{
	RSA_free(instance->keypair);
    BIO_free_all(instance->pub);
    BIO_free_all(instance->pri);
    free(instance->pri_key);
    free(instance->pub_key);
}

struct crypto_message* vpn_decrypt(struct crypto_instance* instance, uint8_t* ciphertext, uint32_t size)
{
	char* decrypt = malloc(30000); /* Fix size of decrypt */
    char* err = malloc(30000);
    int ret = RSA_private_decrypt(size, (unsigned char*)ciphertext, (unsigned char*)decrypt, instance->keypair, RSA_PKCS1_OAEP_PADDING);
    if(ret == -1)
    {
    	ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message: %s\n", err);
    	printf("Something went wrong decrypting message!\n");
    	return NULL;
    }

    struct crypto_message* message = malloc(sizeof(struct crypto_message));
    message->buffer = decrypt;
    message->size = ret;

    return message;
}

struct crypto_message* vpn_encrypt(uint8_t* cleartext, uint32_t size, RSA *myRSA)
{
	char* encrypt = malloc(RSA_size(myRSA));
    int encrypt_len = RSA_public_encrypt(size, (unsigned char*)cleartext, (unsigned char*)encrypt, myRSA, RSA_PKCS1_OAEP_PADDING);
    if(encrypt_len == -1)
    {
    	printf("Something went wrong encrypting message!\n");
    	return NULL;
    }

    struct crypto_message* message = malloc(sizeof(struct crypto_message));
    message->buffer = encrypt;
    message->size = encrypt_len;

    return message;
}