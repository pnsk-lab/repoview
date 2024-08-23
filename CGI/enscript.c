/* $Id$ */

#include "rv_enscript.h"

#include "../config.h"

#include "rv_util.h"
#include "rv_repo.h"

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

char* rv_enscript(const char* repouser, const char* path, const char* lang) {
	if(rv_get_filesize(repouser, path) > 1024 * 128) return NULL;
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	int spipes[2];
	pipe(spipes);
	pid_t pid = fork();
	if(pid == 0) {
		close(spipes[0]);
		dup2(spipes[1], STDOUT_FILENO);
		char* cmd[] = {"svnlook", "cat", svnpath, (char*)path, NULL};
		execvp("svnlook", cmd);
		_exit(0);
	} else {
		close(spipes[1]);
		int pipes[2];
		pipe(pipes);
		pid_t epid = fork();
		if(epid == 0) {
			close(pipes[0]);
			dup2(spipes[0], STDIN_FILENO);
			dup2(pipes[1], STDOUT_FILENO);
			char* hl = rv_strcat("--highlight=", lang);
			char* cmd[] = {"enscript", "-whtml", "--color", "-p", "-", hl, NULL};
			execvp("enscript", cmd);
			_exit(0);
		}
		close(pipes[1]);
		char cbuf[2];
		cbuf[1] = 0;
		char* d = malloc(1);
		d[0] = 0;
		while(1) {
			int n = read(pipes[0], cbuf, 1);
			if(n == 0) break;
			char* tmp = d;
			d = rv_strcat(tmp, cbuf);
			free(tmp);
		}
		close(pipes[0]);
		close(spipes[0]);
		int status;
		bool bad = false;
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			free(d);
			free(svnpath);
			bad = true;
		}
		waitpid(epid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			if(!bad) {
				free(d);
				free(svnpath);
				bad = true;
			}
		}
		if(bad) return NULL;
		free(svnpath);
		if(strlen(d) == 0) {
			free(d);
			return NULL;
		}
		char* newdata = malloc(1);
		newdata[0] = 0;
		int i;
		int incr = 0;
		bool log = false;
		for(i = 0;; i++) {
			if(d[i] == '\n' || d[i] == 0) {
				char oldc = d[i];
				d[i] = 0;

				if(strcmp(d + incr, "<PRE>") == 0) {
					log = true;
				} else if(strcmp(d + incr, "</PRE>") == 0) {
					log = false;
				} else if(log) {
					char* tmp = newdata;
					newdata = rv_strcat3(tmp, d + incr, "\n");
					free(tmp);
				}

				incr = i + 1;
				if(oldc == 0) break;
			}
		}
		free(d);
		return newdata;
	}
}
