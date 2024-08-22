/* $Id$ */

#include "rv_md5.h"

#include <openssl/md5.h>

#include <stdlib.h>
#include <string.h>

char* rv_md5(const char* string) {
	const char hex[] = "0123456789abcdef";
	unsigned char* hash = malloc(MD5_DIGEST_LENGTH);
	MD5((const unsigned char*)string, strlen(string), hash);
	char* str = malloc(128 / 4 + 1);
	int i;
	for(i = 0; i < 128 / 8; i++) {
		str[2 * i + 0] = hex[(hash[i] >> 4) & 0xf];
		str[2 * i + 1] = hex[(hash[i] & 0xf)];
	}
	free(hash);
	return str;
}
