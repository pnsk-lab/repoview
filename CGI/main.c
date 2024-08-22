/* $Id$ */

#include <stdio.h>

#include "../config.h"

#include "rv_sanity.h"
#include "rv_query.h"
#include "rv_page.h"
#include "rv_util.h"
#include "rv_db.h"
#include "rv_auth.h"
#include "rv_multipart.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

char* postdata;

int main() {
	srand(time(NULL));
	rv_check_sanity();
	rv_init_db();
	rv_parse_query(getenv("QUERY_STRING"));
	rv_save_query('Q');
	postdata = malloc(1);
	postdata[0] = 0;
	int hasauth = 0;
	int use_multi = 0;
	char* type = getenv("CONTENT_TYPE");
	if(type == NULL) {
		type = rv_strdup("");
	} else {
		type = rv_strdup(type);
	}
	if(strcmp(type, "application/x-www-form-urlencoded") == 0) {
		char cbuf[2];
		cbuf[1] = 0;
		while(1) {
			fread(cbuf, 1, 1, stdin);
			if(feof(stdin)) break;
			char* tmp = postdata;
			postdata = rv_strcat(tmp, cbuf);
			free(tmp);
		}
	} else {
		int i;
		for(i = 0; type[i] != 0; i++) {
			if(type[i] == ';') {
				type[i] = 0;
				i++;
				bool found = false;
				char* bd = NULL;
				for(; type[i] != 0; i++) {
					if(type[i] != ' ' && type[i] != '\t') {
						int j;
						char* boundary = type + i;
						for(j = 0; boundary[j] != 0; j++) {
							if(boundary[j] == '=') {
								boundary[j] = 0;
								if(strcmp(boundary, "boundary") == 0) {
									bd = rv_strdup(boundary + j + 1);
									found = true;
								}
								break;
							}
						}
						break;
					}
				}
				if(!found) {
					rv_error_http();
					printf("Bad multipart/form-data. Parsing fail.");
					goto freeall;
				} else {
					char* multipart = NULL;
					unsigned long long length = 0;
					char buffer[512];
					while(1) {
						int len = fread(buffer, 1, 512, stdin);
						if(length > 0) {
							char* old = multipart;
							multipart = malloc(length + len);
							int i;
							for(i = 0; i < length; i++) {
								multipart[i] = old[i];
							}
							free(old);
							memcpy(multipart + length, buffer, len);
							length += len;
						} else {
							length += len;
							multipart = malloc(length);
							memcpy(multipart, buffer, len);
						}
						if(feof(stdin)) break;
					}
					if(multipart != NULL) {
						rv_parse_multipart(multipart, bd, length);
						free(multipart);
					}
				}
				if(bd != NULL) free(bd);
				use_multi = 1;
				break;
			}
		}
	}
	if(use_multi == 0) {
		rv_parse_query(postdata);
		rv_save_query('P');
	}
	rv_init_auth();
	hasauth = 1;
	rv_process_page();
	printf("Content-Type: text/html\r\n");
	printf("\r\n");
	rv_print_page();
	if(use_multi == 0) {
		rv_load_query('P');
		rv_free_query();
	}
freeall:
	rv_load_query('Q');
	rv_free_query();
	rv_close_db();
	rv_free_multipart();
	if(hasauth) rv_free_auth();
	free(type);
}
