/* $Id$ */

#include "rv_sha512.h"

#include <openssl/sha.h>

#include <stdlib.h>
#include <string.h>

unsigned char* rv_sha512(const char* string) {
	unsigned char* hash = malloc(SHA512_DIGEST_LENGTH);
	SHA512((const unsigned char*)string, strlen(string), hash);
	return hash;
}
