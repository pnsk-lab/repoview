/* $Id$ */

#include "rv_auth.h"

#include "rv_util.h"
#include "rv_db.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern char** environ;

struct cookie_entry {
	char* key;
	char* value;
};

struct cookie_entry** cookie_entries;

void parse_cookie(void) {
	cookie_entries = malloc(sizeof(*cookie_entries));
	cookie_entries[0] = NULL;
	char* cookie = getenv("HTTP_COOKIE");
	if(cookie != NULL) {
		cookie = rv_strdup(cookie);
		int i;
		int incr = 0;
		for(i = 0;; i++) {
			if(cookie[i] == 0 || cookie[i] == ';') {
				char oldc = cookie[i];
				cookie[i] = 0;

				char* key = cookie + incr;
				char* value = "";

				int j;
				for(j = 0; key[j] != 0; j++) {
					if(key[j] == '=') {
						key[j] = 0;
						value = key + j + 1;
						break;
					}
				}
				struct cookie_entry* entry = malloc(sizeof(*entry));
				entry->key = rv_strdup(key);
				entry->value = rv_strdup(value);

				struct cookie_entry** old_entries = cookie_entries;
				for(j = 0; old_entries[j] != NULL; j++)
					;
				cookie_entries = malloc(sizeof(*cookie_entries) * (j + 2));
				for(j = 0; old_entries[j] != NULL; j++) {
					cookie_entries[j] = old_entries[j];
				}
				cookie_entries[j] = entry;
				cookie_entries[j + 1] = NULL;

				int oldi = i;
				i++;
				for(; cookie[i] != 0 && (cookie[i] == ' ' || cookie[i] == '\t'); i++)
					;
				i--;
				incr = i + 1;
				if(oldc == 0) break;
			}
		}
		free(cookie);
	}
}

char* rv_logged_in(void) {
	int i;
	for(i = 0; cookie_entries[i] != NULL; i++) {
		if(strcmp(cookie_entries[i]->key, "token") == 0) {
			char* who = rv_who_has_token(cookie_entries[i]->value);
			if(who == NULL) {
				printf("Set-Cookie: token=; HttpOnly; Expires=0; SameSite=Strict\r\n");
			}
			return who;
			break;
		}
	}
	return NULL;
}

void rv_logout(void) {
	int i;
	for(i = 0; cookie_entries[i] != NULL; i++) {
		if(strcmp(cookie_entries[i]->key, "token") == 0) {
			printf("Set-Cookie: token=; HttpOnly; Expires=0; SameSite=Strict\r\n");
			rv_remove_token(cookie_entries[i]->value);
			break;
		}
	}
}

void rv_save_login(const char* username) {
	char* token = rv_new_token(username);
	printf("Set-Cookie: token=%s; HttpOnly; SameSite=Strict\r\n", token);
	free(token);
}

void rv_init_auth(void) { parse_cookie(); }

void rv_free_auth(void) {
	int i;
	for(i = 0; cookie_entries[i] != NULL; i++) {
		free(cookie_entries[i]->key);
		free(cookie_entries[i]->value);
		free(cookie_entries[i]);
	}
	free(cookie_entries);
}
