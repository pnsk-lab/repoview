/* $Id$ */

#ifndef __RV_DB_H__
#define __RV_DB_H__

#include <stdbool.h>

void rv_init_db(void);
void rv_close_db(void);
bool rv_has_user(const char* name);
bool rv_check_password(const char* username, const char* password);
void rv_save_token(const char* username, const char* token);
char* rv_who_has_token(const char* token);
bool rv_has_token(const char* token);
void rv_remove_token(const char* token);
void rv_create_user(const char* username, const char* password);

#endif
