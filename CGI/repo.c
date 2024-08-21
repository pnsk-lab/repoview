/* $Id$ */

#include "rv_repo.h"

#include "../config.h"

#include "rv_util.h"

#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

char* rv_construct_repouser(const char* reponame, const char* username) {
	char cbuf[2];
	cbuf[0] = REPO_USER_DELIM;
	cbuf[1] = 0;
	return rv_strcat3(reponame, cbuf, username);
}

bool rv_repo_exists(const char* repouser) {
	char* path = rv_strcat3(SVN_ROOT, "/", repouser);
	if(access(path, F_OK) == 0) {
		free(path);
		return true;
	}
	return false;
}

void rv_repo_list(const char* username, void (*handler)(const char* name, const char* rev)) {
	struct dirent** nl;
	int n = scandir(SVN_ROOT, &nl, NULL, alphasort);
	if(n < 0) return;
	int i;
	for(i = 0; i < n; i++) {
		if(strcmp(nl[i]->d_name, "..") != 0 && strcmp(nl[i]->d_name, ".") != 0) {
			char* tmp = rv_strcat3(SVN_ROOT, "/", nl[i]->d_name);
			char* path = rv_strcat(tmp, "/db/current");
			free(tmp);
			char* str = rv_strdup(nl[i]->d_name);
			int j;
			for(j = 0; str[j] != 0; j++) {
				if(str[j] == REPO_USER_DELIM) {
					str[j] = 0;
					if(strcmp(str + j + 1, username) == 0) {
						struct stat s;
						char* rev = rv_strdup("???");
						if(stat(path, &s) == 0) {
							free(rev);
							rev = malloc(s.st_size + 1);
							FILE* f = fopen(path, "r");
							fread(rev, 1, s.st_size, f);
							fclose(f);
							rev[s.st_size] = 0;
						}
						handler(str, rev);
						free(rev);
					}
					break;
				}
			}
			free(path);
			free(str);
		}
		free(nl[i]);
	}
	free(nl);
}

void null_exec(char** cmd) {
	pid_t pid = fork();
	if(pid == 0) {
		int null = open("/dev/null", O_RDWR);
		dup2(STDOUT_FILENO, null);
		execvp(cmd[0], cmd);
		_exit(0);
	} else {
		waitpid(pid, 0, 0);
	}
}

void rv_create_repo(const char* repouser) {
	char* user = rv_strdup(repouser);
	int i;
	for(i = 0; user[i] != 0; i++) {
		if(user[i] == REPO_USER_DELIM) {
			user[i] = 0;
			break;
		}
	}
	char* path = rv_strcat3(SVN_ROOT, "/", repouser);
	char* cmd[] = {"svnadmin", "create", path, NULL};
	null_exec(cmd);

	char* readme = rv_strcat(path, "/README.txt");
	FILE* f = fopen(readme, "w");
	fprintf(f, "This is the default README of the RepoView repository.\n");
	fprintf(f, "You can change it from the repository manager.\n");
	fclose(f);
	free(readme);

	free(path);
	f = fopen(APACHE_AUTHZ, "r+");
	lockf(fileno(f), F_LOCK, 0);

	fseek(f, 0, SEEK_END);

	fprintf(f, "#%%START %s\n", repouser);
	fprintf(f, "[%s:/]\n", repouser);
	fprintf(f, "* = r\n");
	fprintf(f, "%s = rw\n", user);
	fprintf(f, "#%%END\n");

	lockf(fileno(f), F_ULOCK, 0);
	free(user);
}

char* rv_get_readme(const char* repouser) {
	char* tmp = rv_strcat3(SVN_ROOT, "/", repouser);
	char* path = rv_strcat(tmp, "/README.txt");
	free(tmp);
	struct stat s;
	if(stat(path, &s) == 0) {
		FILE* f = fopen(path, "r");
		char* buf = malloc(s.st_size + 1);
		fread(buf, 1, s.st_size, f);
		fclose(f);
		buf[s.st_size] = 0;
		return buf;
	}
	return NULL;
}

long long rv_get_filesize(const char* repouser, const char* path) {
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	int pipes[2];
	pipe(pipes);
	pid_t pid = fork();
	if(pid == 0) {
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		char* cmd[] = {"svnlook", "filesize", svnpath, (char*)path, NULL};
		execvp("svnlook", cmd);
		_exit(0);
	} else {
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
		int status;
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			free(d);
			free(svnpath);
			return -1;
		}
		long long sz = atoll(d);
		free(svnpath);
		free(d);
		return sz;
	}
}

void rv_remove_repo(const char* repouser) {
	printf("");
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	pid_t pid = fork();
	if(pid == 0) {
		char* cmd[] = {"rm", "-rf", svnpath, NULL};
		execvp("rm", cmd);
		_exit(0);
	} else {
		waitpid(pid, 0, 0);
	}
	free(svnpath);

	FILE* f = fopen(APACHE_AUTHZ, "r+");
	lockf(fileno(f), F_LOCK, 0);

	fseek(f, 0, SEEK_SET);
	struct stat s;
	stat(APACHE_AUTHZ, &s);
	char* buffer = malloc(s.st_size + 1);
	fread(buffer, 1, s.st_size, f);
	buffer[s.st_size] = 0;

	f = freopen(APACHE_AUTHZ, "w+", f);
	lockf(fileno(f), F_LOCK, 0);
	int incr = 0;
	int i;
	char* start = rv_strcat("#%START ", repouser);
	bool discard = false;
	for(i = 0;; i++) {
		if(buffer[i] == '\n' || buffer[i] == 0) {
			char oldc = buffer[i];
			buffer[i] = 0;

			char* line = buffer + incr;
			if(strcmp(line, start) == 0) {
				discard = true;
			} else if(discard && strcmp(line, "#%END") == 0) {
				discard = false;
			} else if(strcmp(line, "") == 0) {
			} else if(!discard) {
				fprintf(f, "%s\n", line);
			}

			incr = i + 1;
			if(oldc == 0) break;
		}
	}

	lockf(fileno(f), F_ULOCK, 0);
	fclose(f);
}

void rv_set_readme(const char* repouser, const char* readme) {
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	char* path = rv_strcat(svnpath, "/README.txt");
	FILE* f = fopen(path, "w");
	if(f != NULL) {
		fwrite(readme, 1, strlen(readme), f);
		fclose(f);
	}
	free(path);
	free(svnpath);
}

bool rv_get_list(const char* repouser, const char* path, void (*handler)(const char* pathname), int* isdir) {
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	int pipes[2];
	*isdir = 0;
	pipe(pipes);
	pid_t pid = fork();
	if(pid == 0) {
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		char* cmd[] = {"svnlook", "-N", "tree", svnpath, (char*)path, NULL};
		execvp("svnlook", cmd);
		_exit(0);
	} else {
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
		int status;
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			free(d);
			free(svnpath);
			return false;
		}
		int count = 0;
		int incr = 0;
		int i;
		int phase = 0;
	repeat:
		for(i = 0;; i++) {
			if(d[i] == '\r') {
				d[i] = 0;
			} else if(d[i] == '\n' || d[i] == 0) {
				char oldc = d[i];
				d[i] = 0;
				count++;
				if(count > 1 && d[incr] != 0 && strlen(d + incr + 1) > 0) {
					char* pathname = d + incr + 1;
					if(phase == 0 && pathname[strlen(pathname) - 1] == '/') {
						handler(d + incr + 1);
					} else if(phase == 1 && pathname[strlen(pathname) - 1] != '/') {
						handler(d + incr + 1);
					}
				} else {
					char* pathname = d + incr;
					if(pathname[strlen(pathname) - 1] == '/') *isdir = 1;
				}
				d[i] = oldc;
				incr = i + 1;
				if(oldc == 0) break;
			}
		}
		phase++;
		incr = 0;
		count = 0;
		if(phase == 1) goto repeat;
		free(d);
	}
	free(svnpath);
	return true;
}

char* rv_read_file(const char* repouser, char* path) {
	if(rv_get_filesize(repouser, path) > 1024 * 128) return NULL;
	char* svnpath = rv_strcat3(SVN_ROOT, "/", repouser);
	int pipes[2];
	pipe(pipes);
	pid_t pid = fork();
	if(pid == 0) {
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		char* cmd[] = {"svnlook", "cat", svnpath, (char*)path, NULL};
		execvp("svnlook", cmd);
		_exit(0);
	} else {
		close(pipes[1]);
		char cbuf[1024];
		char* d = malloc(1024 * 128);
		int incr = 0;
		bool bin = false;
		while(1) {
			int n = read(pipes[0], cbuf, 1024);
			if(cbuf[0] == 0) {
				bin = true;
				kill(pid, SIGKILL);
				break;
			}
			if(n == 0) break;
			memcpy(d + incr, cbuf, n);
			d[incr + n] = 0;
			incr += n;
		}
		int status;
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			free(d);
			free(svnpath);
			return NULL;
		}
		if(bin) {
			free(d);
			return NULL;
		}
		return d;
	}
}
