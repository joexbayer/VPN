#include "../includes/crypto.h"

 /* A 128 bit IV */
const unsigned char IV[] = "0123456789012345";

    /* Some additional data to be authenticated */
const unsigned char aad[] = "Some AAD data";

/** crypto_instance
 * Used to initilize crypto context for server
 * @void: 
 * 
 * Creates entire crypto context including RSA public priate keys
 * with each text version aswell.
 * 
 * Returns created crypto instance
 */
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

/** free_crypto_instance
 * Frees all memory that is used by crypto
 * @instance: crypto instance to free.
 * 
 * Frees RSA, BIO and char*'S
 *  returns void.
 */
void free_crypto_instance(struct crypto_instance* instance)
{
	RSA_free(instance->keypair);
    BIO_free_all(instance->pub);
    BIO_free_all(instance->pri);
    free(instance->pri_key);
    free(instance->pub_key);
}


/** vpn_rsa_decrypt
 * Decrypts given ciphertext based on crypto instance.
 * 
 * @instance: instance to use to decrypt
 * @ciphertext: text to decrypt
 * @size: size of ciphertext
 * 
 * uses RSA crypto instance to decrypt a message.
 * Creates a crypto message as reponse.
 * 
 * Returns created crypto message.
 */
struct crypto_message* vpn_rsa_decrypt(struct crypto_instance* instance, uint8_t* ciphertext, uint32_t size)
{
	char* decrypt = malloc(2555); /* Fix size of decrypt */
    char* err = malloc(100);
    int ret = RSA_private_decrypt(size, (unsigned char*)ciphertext, (unsigned char*)decrypt, instance->keypair, RSA_PKCS1_OAEP_PADDING);
    if(ret == -1)
    {
    	ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message: %s\n", err);
    	printf("Something went wrong decrypting message!\n");
    	free(err);
    	return NULL;
    }

    free(err);
    struct crypto_message* message = malloc(sizeof(struct crypto_message));
    message->buffer = decrypt;
    message->size = ret;

    return message;
}

/** vpn_rsa_encrypt
 * Encrypts given cleartext based on public RSA key
 * 
 * @cleartext: text to encrypt
 * @size: size of cleartext
 * @myRSA: public key context
 * 
 * uses RSA public key to encrypt a message.
 * Creates a crypto message as reponse.
 * 
 * Returns created crypto message.
 */
struct crypto_message* vpn_rsa_encrypt(uint8_t* cleartext, uint32_t size, RSA *myRSA)
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

/** vpn_aes_encrypt
 * Encrypts given plaintext based on AES key
 * 
 * @plaintext: text to encrypt
 * @plaintext_len: size of plaintext
 * @aad: additional authenticated data 
 * @add_len: length of additional authenticated data
 * @key: key to use for decryption
 * @iv: IV for decryption
 * @ciphertext: result of encryption
 * @tag: tag for integrity
 * 
 * Uses AES encryption on given plaintext
 * important is that TAG is correct
 * 
 * Returns size of ciphertext or -1
 */
int vpn_aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad, int aad_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, unsigned char *tag)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, ciphertext_len = 0;

    /* Create and initialise the context */
    ctx = EVP_CIPHER_CTX_new();

    /* Initialise the encryption operation. */
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

    /* Set IV length if default 12 bytes (96 bits) is not appropriate */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL);

    /* Initialise key and IV */
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if(aad && aad_len > 0)
    {
        EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len);
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(plaintext)
    {
        EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
        ciphertext_len = len;
    }

    /* Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    /* Get the tag */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}


/** vpn_aes_decrypt
 * Decrypts given ciphertext based on AES key
 * 
 * @ciphertext: text to decrypt
 * @ciphertext_len: size of ciphertext
 * @aad: additional authenticated data 
 * @add_len: length of additional authenticated data
 * @tag: integrity tag
 * @key: key to use for decryption
 * @iv: IV for decryption
 * @plaintext: result put in plaintext
 * 
 * Uses AES decryption on given ciphertext
 * important is that TAG is correct
 * 
 * Returns size of plaintext or -1
 */
int vpn_aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad, int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, plaintext_len = 0, ret;

    /* Create and initialise the context */
    ctx = EVP_CIPHER_CTX_new();

    /* Initialise the decryption operation. */
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL);

    /* Initialise key and IV */
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if(aad && aad_len > 0)
    {
        EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len);
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if(ciphertext)
    {
        EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);

        plaintext_len = len;
    }

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);

    /* Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0)
    {
        /* Success */
        plaintext_len += len;
        return plaintext_len;
    }
    else
    {
        /* Verify failed */
        return -1;
    }
}