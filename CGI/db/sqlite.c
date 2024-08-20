/* $Id$ */

#include "rv_db.h"

#include "../../config.h"

#include "rv_util.h"

#include <sqlite3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

sqlite3* sql;

void rv_init_db(void){
	int ret;
	ret = sqlite3_open(DB_ROOT "/db.sqlite3", &sql);
	if(ret != SQLITE_OK){
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
	char* err;
	ret = sqlite3_exec(
		sql,
		"create table if not exists users(user text, password text)",
		NULL,
		NULL,
		&err
	);
	if(ret != SQLITE_OK){
		sqlite3_free(err);
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
	ret = sqlite3_exec(
		sql,
		"create table if not exists tokens(user text, token text)",
		NULL,
		NULL,
		&err
	);
	if(ret != SQLITE_OK){
		sqlite3_free(err);
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
}

void rv_close_db(void){
	sqlite3_close(sql);
}

int count = 0;

int sqlcount(void* param, int ncol, char** row, char** col){
	count = ncol;
	fprintf(stderr, "%d\n", ncol);
	return 0;
}

bool rv_has_user(const char* username){
	char* err;
	char cbuf[2];
	cbuf[1] = 0;
	char* query = rv_strdup("select * from users where user = '");
	int i;
	for(i = 0; username[i] != 0; i++){
		if(username[i] == '\''){
			cbuf[0] = username[i];
			char* tmp = query;
			tmp = rv_strcat(tmp, cbuf);
			free(tmp);
			cbuf[0] = username[i];
			tmp = query;
			query = rv_strcat(tmp, cbuf);
			free(tmp);
		}else{
			cbuf[0] = username[i];
			char* tmp = query;
			query = rv_strcat(tmp, cbuf);
			free(tmp);
		}
	}
	char* tmp = query;
	query = rv_strcat(tmp, "'");
	free(tmp);
	int ret;
	fprintf(stderr, "%s\n", query);
	ret = sqlite3_exec(
		sql,
		query,
		sqlcount,
		NULL,
		&err
	);
	free(query);
	if(ret != SQLITE_OK){
		sqlite3_free(err);
	}
	return count > 0;
}
