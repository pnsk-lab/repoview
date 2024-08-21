/* $Id$ */

#ifndef __RV_UTIL_H__
#define __RV_UTIL_H__

#ifdef __MINGW32__
#define PATH_DELIM ';'
#else
#define PATH_DELIM ':'
#endif

char* rv_strcat(const char* a, const char* b);
char* rv_strcat3(const char* a, const char* b, const char* c);
char* rv_strdup(const char* str);
char* rv_url_decode(const char* str);
void rv_error_http(void);
char* rv_new_token(const char* username);
char* rv_construct_repouser(const char* reponame, const char* username);

#endif
