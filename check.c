/* $Id$ */

#include "config.h"

#include <stdio.h>

int check_db(void) {
	int counter = 0;
	const char* db = "";
#ifdef USE_SQLITE
	counter++;
	db = "SQLite";
#endif
#ifdef USE_GDBM
	counter++;
	db = "GDBM";
#endif
#ifdef USE_NDBM
	counter++;
	db = "NDBM";
#endif
	if(counter > 1) {
		fprintf(stderr, "You cannot use multiple database types at once.\n");
		return 1;
	} else if(counter == 0) {
		fprintf(stderr, "You must select a database type.\n");
		return 1;
	} else {
		printf("Database type is %s\n", db);
	}
	return 0;
}

int check_theme(void) {
	int counter = 0;
	const char* theme = "";
#ifdef USE_MODERN
	counter++;
	theme = "Modern";
#endif
#ifdef USE_OPTIMIZED
	counter++;
	theme = "Optimized";
#endif
	if(counter > 1) {
		fprintf(stderr, "You cannot use multiple themes at once.\n");
		return 1;
	} else if(counter == 0) {
		fprintf(stderr, "You must select a theme.\n");
		return 1;
	} else {
		printf("Theme is %s\n", theme);
	}
	return 0;
}

int check_auth(void) {
	int counter = 0;
	const char* method = "";
#ifdef USE_COOKIE
	counter++;
	method = "Cookie";
#endif
	if(counter > 1) {
		fprintf(stderr, "You cannot use multiple authentication methods at once.\n");
		return 1;
	} else if(counter == 0) {
		fprintf(stderr, "You must select an authentication method.\n");
		return 1;
	} else {
		printf("Authentication method is %s\n", method);
	}
	return 0;
}

int main() {
	int st;
	st = check_db();
	if(st != 0) goto fail;
	st = check_auth();
	if(st != 0) goto fail;
	st = check_theme();
	if(st != 0) goto fail;
	printf("Config validation successful.\n");
	return 0;
fail:
	fprintf(stderr, "Config validation failure.\n");
	return st;
}
