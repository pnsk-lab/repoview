/* $Id$ */

#include "rv_query.h"

#include "rv_util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct query_entry {
	char* key;
	char* value;
};

struct query_entry** qentries;

struct query_entry** query;
struct query_entry** postquery;

void rv_save_query(char c) {
	if(c == 'Q') {
		query = qentries;
	} else if(c == 'P') {
		postquery = qentries;
	}
}

void rv_load_query(char c) {
	if(c == 'Q') {
		qentries = query;
	} else if(c == 'P') {
		qentries = postquery;
	}
}

void rv_parse_query(const char* oldquery) {
	char* query = rv_strdup(oldquery);
	int i;
	int incr = 0;
	qentries = malloc(sizeof(*qentries));
	qentries[0] = NULL;
	for(i = 0;; i++) {
		if(query[i] == '&' || query[i] == 0) {
			char oldc = query[i];
			query[i] = 0;

			char* key = query + incr;
			char* value = "";

			int j;
			for(j = 0; key[j] != 0; j++) {
				if(key[j] == '=') {
					key[j] = 0;
					value = key + j + 1;
					break;
				}
			}

			struct query_entry* entry = malloc(sizeof(*entry));
			entry->key = rv_url_decode(key);
			entry->value = rv_url_decode(value);

			struct query_entry** old_entries = qentries;
			for(j = 0; old_entries[j] != NULL; j++)
				;
			qentries = malloc(sizeof(*qentries) * (j + 2));
			for(j = 0; old_entries[j] != NULL; j++) {
				qentries[j] = old_entries[j];
			}
			qentries[j] = entry;
			qentries[j + 1] = NULL;
			free(old_entries);

			incr = i + 1;
			if(oldc == 0) break;
		}
	}
	free(query);
}

void rv_free_query(void) {
	int i;
	for(i = 0; qentries[i] != NULL; i++) {
		free(qentries[i]->key);
		free(qentries[i]->value);
		free(qentries[i]);
	}
	free(qentries);
}

char* rv_get_query(const char* key) {
	int i;
	for(i = 0; qentries[i] != NULL; i++) {
		if(strcmp(qentries[i]->key, key) == 0) {
			return qentries[i]->value;
		}
	}
	return NULL;
}
