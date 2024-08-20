/* $Id$ */

#include "rv_sha512.h"

#include <openssl/sha.h>

#include <stdlib.h>
#include <string.h>

char* rv_sha512(const char* string) {
	const char hex[] = "0123456789abcdef";
	unsigned char* hash = malloc(SHA512_DIGEST_LENGTH);
	SHA512((const unsigned char*)string, strlen(string), hash);
	char* str = malloc(512 / 4 + 1);
	int i;
	for(i = 0; i < 512 / 8; i++) {
		str[2 * i + 0] = hex[(hash[i] >> 4) & 0xf];
		str[2 * i + 1] = hex[(hash[i] & 0xf)];
	}
	free(hash);
	return str;
}
