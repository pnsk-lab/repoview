/* $Id$ */

#include "rv_sanity.h"

#include "rv_version.h"
#include "rv_util.h"
#include "../config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

bool rv_find_executable(const char* name) {
#ifdef USE_PATH
	char* path = rv_strcat(USE_PATH, "");
#else
	char* path = rv_strcat(getenv("PATH"), "");
#endif
	int i;
	int incr = 0;
	for(i = 0;; i++) {
		if(path[i] == 0 || path[i] == PATH_DELIM) {
			char oldc = path[i];
			path[i] = 0;
			char* exec = rv_strcat3(path + incr, "/", name);
#ifdef __MINGW32__
			char* tmp = exec;
			exec = rv_strcat(exec, ".exe");
			free(tmp);
#endif
			if(access(exec, F_OK) == 0) {
				free(exec);
				free(path);
				return true;
			}
			free(exec);
			incr = i + 1;
			if(oldc == 0) break;
		}
	}
	free(path);
	return false;
}

void rv_check_sanity(void) {
	bool sane = true;

	bool svnlook = rv_find_executable("svnlook");
	bool svnadmin = rv_find_executable("svnadmin");
	bool htpasswd = rv_find_executable("htpasswd");
#ifdef USE_ENSCRIPT
	bool enscript = rv_find_executable("enscript");
#endif
	bool rm = rv_find_executable("rm");

	if(!svnlook) sane = false;
	if(!svnadmin) sane = false;
	if(!htpasswd) sane = false;
#ifdef USE_ENSCRIPT
	if(!enscript) sane = false;
#endif
	if(!rm) sane = false;

	if(!sane) {
		rv_error_http();
		if(!svnlook) printf("svnlook not found\n");
		if(!svnadmin) printf("svnadmin not found\n");
		if(!htpasswd) printf("htpasswd not found\n");
#ifdef USE_ENSCRIPT
		if(!enscript) printf("enscript not found\n");
#endif
		if(!rm) printf("rm not found\n");
		exit(1);
	}
}
