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
	multipart_entries = malloc(sizeof(*multipart_entries));
	multipart_entries[0] = NULL;
	char* name = NULL;
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
					struct multipart_entry* entry = malloc(sizeof(*entry));
					entry->length = datalen;
					entry->data = malloc(datalen);
					entry->name = rv_strdup(name);
					unsigned long long j;
					for(j = 0; j < datalen; j++) entry->data[j] = data[j];

					struct multipart_entry** old_entries = multipart_entries;
					for(j = 0; old_entries[j] != NULL; j++)
						;
					multipart_entries = malloc(sizeof(*multipart_entries) * (j + 2));
					for(j = 0; old_entries[j] != NULL; j++) {
						multipart_entries[j] = old_entries[j];
					}
					multipart_entries[j] = entry;
					multipart_entries[j + 1] = NULL;
					free(old_entries);
				}
				phase = 0;
				if(strcmp(eb, line) == 0) {
					free(line);
					break;
				}
			} else if(phase == 0) {
				line[strlen(line) - 1] = 0;
				int j;
				for(j = 0; line[j] != 0; j++) {
					if(line[j] == ':') {
						line[j] = 0;
						char* value = "";
						j++;
						for(; line[j] != 0; j++) {
							if(line[j] != ' ' && line[j] != '\t') {
								value = line + j;
								break;
							}
						}
						if(strcasecmp(line, "Content-Disposition") == 0) {
							int j;
							int incrval = 0;
							for(j = 0;; j++) {
								if(value[j] == ';' || value[j] == 0) {
									char oldc = value[j];
									value[j] = 0;
									char* kv = value + incrval;
									j++;
									for(; value[j] != 0; j++) {
										if(value[j] != ' ' && value[j] != '\t') break;
									}
									incrval = j;
									int k;
									for(k = 0; kv[k] != 0; k++) {
										if(kv[k] == '=') {
											kv[k] = 0;
											if(strcmp(kv, "name") == 0) {
												if(name != NULL) free(name);
												name = malloc(strlen(kv + k + 1) + 1);
												char* nkv = kv + k + 1;
												int l = 0;
												int incrn = 0;
												for(l = 0; nkv[l] != 0; l++) {
													if(nkv[l] != '"') {
														name[incrn++] = nkv[l];
													}
												}
												name[incrn] = 0;
											}
											break;
										}
									}
									if(oldc == 0) break;
								}
							}
						}
						break;
					}
				}
			}
			free(line);
			incr = i + 1;
			if(i == length) break;
		}
	}
	if(name != NULL) free(name);
	free(b);
}
