/* $Id$ */

#ifndef __RV_AUTH_H__
#define __RV_AUTH_H__

#include <stdbool.h>

char* rv_logged_in(void);
void rv_free_auth(void);
void rv_init_auth(void);
void rv_save_login(const char* username);
void rv_logout(void);

#endif
