#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>

/*
@compare: original un-hashed string
@hashed: the hashed version string 65 bytes including null terminator
*/
int util_compare_hash(const char *compare, const char *hashed) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    char hexresult[SHA256_DIGEST_LENGTH * 2 + 1];

    SHA256((unsigned char *)compare, strlen(compare), result);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hexresult[i * 2], "%02x", result[i]);
    }
    hexresult[SHA256_DIGEST_LENGTH * 2] = '\0';

    if (strcmp(hexresult, hashed) == 0) {
        return 0;
    } else {
        return 1;
    }
}

int util_hash(const char *to_hash, char *hashed) {
    printf("ok\n");
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, to_hash, strlen(to_hash));
    SHA256_Final(hash, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hashed[i * 2], "%02x", hash[i]);
    }
    hashed[SHA256_DIGEST_LENGTH * 2] = '\0';
    printf("hashed %s\n", hashed);
    return 0;
}

int util_generate_token(char token[33]) {
    unsigned char buffer[33 / 2];

    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {

        return -1;
    }

    for (int i = 0; i < sizeof(buffer); i++) {
        sprintf(&token[i * 2], "%02x", buffer[i]);
    }
    token[33 - 1] = '\0';
    return 0;
}
