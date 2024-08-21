/* $Id$ */

#include "rv_db.h"

#include "../../config.h"

#include "rv_util.h"
#include "rv_sha512.h"

#include <sqlite3.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

sqlite3* sql;

void rv_init_db(void) {
	int ret;
	ret = sqlite3_open(DB_ROOT "/db.sqlite3", &sql);
	if(ret != SQLITE_OK) {
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
	char* err;
	ret = sqlite3_exec(sql, "create table if not exists users(user text, password text)", NULL, NULL, &err);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
	ret = sqlite3_exec(sql, "create table if not exists tokens(user text, token text)", NULL, NULL, &err);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
		rv_error_http();
		printf("SQLite3 database error\n");
		exit(1);
	}
}

void rv_close_db(void) { sqlite3_close(sql); }

int count = 0;

struct user {
	char* username;
	char* password;
	bool valid;
};

int sqlcount(void* param, int ncol, char** row, char** col) {
	count = ncol;
	return 0;
}

int sqlgetpasswd(void* param, int ncol, char** row, char** col) {
	struct user* user = (struct user*)param;
	if(strcmp(row[0], user->username) == 0) {
		char* hash = rv_sha512(user->password);
		if(strcmp(row[1], hash) == 0) {
			user->valid = true;
		}
		free(hash);
	}
	return 0;
}

char* escape_sql(const char* input) {
	char* query = malloc(1);
	query[0] = 0;
	char cbuf[2];
	cbuf[1] = 0;
	int i;
	for(i = 0; input[i] != 0; i++) {
		if(input[i] == '\'') {
			cbuf[0] = input[i];
			char* tmp = query;
			query = rv_strcat(tmp, cbuf);
			free(tmp);
			cbuf[0] = input[i];
			tmp = query;
			query = rv_strcat(tmp, cbuf);
			free(tmp);
		} else {
			cbuf[0] = input[i];
			char* tmp = query;
			query = rv_strcat(tmp, cbuf);
			free(tmp);
		}
	}
	return query;
}

bool rv_check_password(const char* username, const char* password) {
	char* err;
	int ret;
	struct user user;
	user.username = (char*)username;
	user.password = (char*)password;
	user.valid = false;
	char* esc = escape_sql(username);
	char* query = rv_strcat3("select * from users where user = '", esc, "'");
	free(esc);
	ret = sqlite3_exec(sql, query, sqlgetpasswd, (void*)&user, &err);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
	return user.valid;
}

void rv_create_user(const char* username, const char* password) {
	char* err;
	int ret;
	char* sha512 = rv_sha512(password);
	char* esc = escape_sql(username);
	char* tmp = rv_strcat3("insert into users values('", esc, "', '");
	char* query = rv_strcat3(tmp, sha512, "')");
	free(sha512);
	free(tmp);
	free(esc);
	ret = sqlite3_exec(sql, query, NULL, NULL, &err);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
	rv_add_auth(username, password);
}

void rv_save_token(const char* username, const char* token) {
	char* err;
	int ret;
	char* esc = escape_sql(username);
	char* tmp = rv_strcat3("insert into tokens values('", esc, "', '");
	char* query = rv_strcat3(tmp, token, "')");
	free(tmp);
	free(esc);
	ret = sqlite3_exec(sql, query, NULL, NULL, &err);
	free(query);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
}

char* has_username;

int sqlget(void* param, int ncol, char** row, char** col) {
	has_username = rv_strdup(row[0]);
	return 0;
}

char* rv_who_has_token(const char* token) {
	char* err;
	count = 0;
	char* query = rv_strcat3("select * from tokens where token = '", token, "'");
	int ret;
	has_username = NULL;
	ret = sqlite3_exec(sql, query, sqlget, (void*)token, &err);
	free(query);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
		return NULL;
	}
	return has_username;
}

bool rv_has_token(const char* token) {
	char* err;
	char* query = rv_strcat3("select * from tokens where token = '", token, "'");
	int ret;
	count = 0;
	ret = sqlite3_exec(sql, query, sqlcount, NULL, &err);
	free(query);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
	return count > 0;
}

void rv_remove_token(const char* token) {
	char* err;
	char* query = rv_strcat3("delete from tokens where token = '", token, "'");
	int ret;
	ret = sqlite3_exec(sql, query, sqlcount, NULL, &err);
	free(query);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
}

bool rv_has_user(const char* username) {
	char* err;
	count = 0;
	char* esc = escape_sql(username);
	char* query = rv_strcat3("select * from users where user = '", esc, "'");
	free(esc);
	int ret;
	ret = sqlite3_exec(sql, query, sqlcount, NULL, &err);
	free(query);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}
	return count > 0;
}
