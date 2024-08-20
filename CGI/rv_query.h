/* $Id$ */

#ifndef __RV_QUERY_H__
#define __RV_QUERY_H__

void rv_parse_query(const char* query);
void rv_free_query(void);
char* rv_get_query(const char* key);

void rv_save_query(char c);
void rv_load_query(char c);

#endif
