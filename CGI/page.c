/* $Id$ */

#include "rv_page.h"

#include "rv_query.h"
#include "rv_util.h"
#include "rv_auth.h"

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>

char* buffer;
char* user;

void render_page(void);

void add_data(char** data, const char* txt) {
	char* tmp = *data;
	*data = rv_strcat(tmp, txt);
	free(tmp);
}

void rv_process_page(void) {
	buffer = malloc(1);
	buffer[0] = 0;
	user = rv_logged_in();
	render_page();
}

void rv_print_page(void) { printf("%s\n", buffer); }
