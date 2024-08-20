/* $Id$ */

#include "rv_util.h"

#include "../config.h"

#include "rv_version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* rv_strcat(const char* a, const char* b){
	char* str = malloc(strlen(a) + strlen(b) + 1);
	memcpy(str, a, strlen(a));
	memcpy(str + strlen(a), b, strlen(b));
	str[strlen(a) + strlen(b)] = 0;
	return str;
}

char* rv_strcat3(const char* a, const char* b, const char* c){
	char* tmp = rv_strcat(a, b);
	char* str = rv_strcat(tmp, c);
	free(tmp);
	return str;
}

char* rv_strdup(const char* str){
	return rv_strcat(str, "");
}

void rv_error_http(void){
	printf("Content-Type: text/plain\r\n");
	printf("Status: 500 Internal Server Error\r\n");
	printf("\r\n");
	printf("This is RepoView version %s, named `%s`.\n", rv_get_version(), INSTANCE_NAME);
	printf("Unrecoverable error has occured.\n");
	printf("Admin contact: %s\n", INSTANCE_ADMIN);
	printf("Developer contact: Nishi <nishi@nishi.boats>\n");
	printf("-----\n");
}

int hex_to_num(char c){
	if('0' <= c && c <= '9'){
		return c - '0';
	}else if('a' <= c && c <= 'f'){
		return c - 'a' + 10;
	}else if('A' <= c && c <= 'F'){
		return c - 'A' + 10;
	}
	return 0;
}

char* rv_url_decode(const char* str){
	char* r = malloc(1);
	r[0] = 0;
	int i;
	char cbuf[2];
	cbuf[1] = 0;
	for(i = 0; str[i] != 0; i++){
		if(str[i] == '%'){
			if(str[i + 1] == 0) break;
			if(str[i + 2] == 0) break;
			cbuf[0] = (hex_to_num(str[i + 1]) << 4) | hex_to_num(str[i + 2]);
			char* tmp = r;
			r = rv_strcat(tmp, cbuf);
			free(tmp);
			i += 2;
		}else{
			cbuf[0] = str[i];
			char* tmp = r;
			r = rv_strcat(tmp, cbuf);
			free(tmp);
		}
	}
	return r;
}
