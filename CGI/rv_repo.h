/* $Id$ */

#ifndef __RV_REPO_H__
#define __RV_REPO_H__

#include <stdbool.h>

char* rv_construct_repouser(const char* reponame, const char* username);
bool rv_repo_exists(const char* repouser);
void rv_repo_list(const char* username, void (*handler)(const char* name, const char* rev));
void rv_create_repo(const char* repouser);
char* rv_get_readme(const char* repouser);
bool rv_get_list(const char* repouser, const char* path, void (*handler)(const char* pathname), int* isdir);
char* rv_read_file(const char* repouser, char* path);
long long rv_get_filesize(const char* repouser, const char* path);
void rv_remove_repo(const char* repouser);

#endif
