/* $Id$ */

#include "rv_multipart.h"

#include "rv_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct multipart_entry** multipart_entries = NULL;

void rv_free_multipart(void) {
	if(multipart_entries == NULL) return;
	int i;
	for(i = 0; multipart_entries[i] != NULL; i++) {
		free(multipart_entries[i]->name);
		free(multipart_entries[i]->data);
		free(multipart_entries[i]);
	}
	free(multipart_entries);
}

struct multipart_entry* rv_get_multipart(const char* name) {
	if(multipart_entries == NULL) return NULL;
	int i;
	for(i = 0; multipart_entries[i] != NULL; i++) {
		if(strcmp(multipart_entries[i]->name, name) == 0) {
			return multipart_entries[i];
		}
	}
	return NULL;
}

void rv_parse_multipart(unsigned char* buffer, char* boundary, unsigned long long length) {
	unsigned long long i;
	int incr = 0;
	int phase = 0;
	unsigned long long fstart = 0;
	char* b = rv_strcat3("--", boundary, "\r");
	char* eb = rv_strcat3("--", boundary, "--\r");
	for(i = 0;; i++) {
		if(i == length || buffer[i] == '\n') {
			char* line = malloc(i - incr + 1);
			memcpy(line, buffer + incr, i - incr);
			line[i - incr] = 0;
			if(line[0] == '\r' && phase == 0) {
				phase = 1;
				fstart = i + 1;
			} else if(strcmp(b, line) == 0 || strcmp(eb, line) == 0) {
				if(phase == 1) {
					unsigned long long fend = i - strlen(line) - 2;
					char* data = buffer + fstart;
					unsigned long long datalen = fend - fstart;
				}
				phase = 0;
				if(strcmp(eb, line) == 0) {
					free(line);
					break;
				}
			} else if(phase == 0) {
				line[strlen(line) - 1] = 0;
				fprintf(stderr, "%s\n", line);
			}
			free(line);
			incr = i + 1;
			if(i == length) break;
		}
	}
	free(b);
}
