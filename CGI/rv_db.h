/* $Id$ */

#ifndef __RV_DB_H__
#define __RV_DB_H__

#include <stdbool.h>

void rv_init_db(void);
void rv_close_db(void);
bool rv_has_user(const char* name);

#endif
