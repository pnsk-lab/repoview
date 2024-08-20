/* $Id$ */

#include "rv_query.h"

#include "rv_util.h"
#include "rv_version.h"
#include "rv_auth.h"
#include "rv_db.h"

#include "../../config.h"

#include <stdlib.h>
#include <string.h>

extern char* buffer;
void add_data(char** data, const char* txt);
void render_stuff();

char* title = NULL;
char* desc = NULL;
char* page = NULL;

void render_page(void) {
	rv_load_query('Q');
	char* query = rv_get_query("page");
	if(query == NULL) query = "welcome";

	if(strcmp(query, "welcome") == 0) {
		title = rv_strdup("Welcome");
		desc = rv_strdup("Welcome to " INSTANCE_NAME ".");
		page = rv_strcat3("Welcome to " INSTANCE_NAME ".<br>This instance is running RepoView version ", rv_get_version(), ".");
	} else if(strcmp(query, "login") == 0) {
		title = rv_strdup("Login");
		desc = rv_strdup("You can log in to your account here.");
		page = rv_strdup("");

		add_data(&page, "<form action=\"");
		add_data(&page, INSTANCE_ROOT);
		add_data(&page, "/?page=sendlogin\" method=\"POST\">\n");
		add_data(&page, "	<table border=\"0\">\n");
		add_data(&page, "		<tr>\n");
		add_data(&page, "			<th>Username</th>\n");
		add_data(&page, "			<td>\n");
		add_data(&page, "				<input name=\"username\">\n");
		add_data(&page, "			</td>\n");
		add_data(&page, "		</tr>\n");
		add_data(&page, "		<tr>\n");
		add_data(&page, "			<th>Password</th>\n");
		add_data(&page, "			<td>\n");
		add_data(&page, "				<input name=\"password\" type=\"password\">\n");
		add_data(&page, "			</td>\n");
		add_data(&page, "		</tr>\n");
		add_data(&page, "	</table>\n");
		add_data(&page, "	<input type=\"submit\" value=\"Login\">\n");
		add_data(&page, "</form>\n");
	} else if(strcmp(query, "sendlogin") == 0) {
		title = rv_strdup("Login Result");
		page = rv_strdup("");

		rv_load_query('P');
		if(rv_get_query("username") == NULL || rv_get_query("password") == NULL) {
			add_data(&page, "Invalid form\n");
		} else {
			if(rv_has_user(rv_get_query("username"))) {
			} else {
				add_data(&page, "User does not exist");
			}
		}
	}

	if(title == NULL) title = rv_strdup("");
	if(desc == NULL) desc = rv_strdup("");
	if(page == NULL) page = rv_strdup("");
	render_stuff();
	free(page);
	free(desc);
	free(title);
}

char* escape(const char* str) {
	char* r = malloc(1);
	r[0] = 0;
	char cbuf[2];
	cbuf[1] = 0;
	int i;
	for(i = 0; str[i] != 0; i++) {
		if(str[i] == '<') {
			char* tmp = r;
			r = rv_strcat(tmp, "&lt;");
			free(tmp);
		} else if(str[i] == '>') {
			char* tmp = r;
			r = rv_strcat(tmp, "&gt;");
			free(tmp);
		} else {
			cbuf[0] = str[i];
			char* tmp = r;
			r = rv_strcat(tmp, cbuf);
			free(tmp);
		}
	}
	return r;
}

void render_stuff(void) {
	char* escaped;
	add_data(&buffer, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n");
	add_data(&buffer, "<html>\n");
	add_data(&buffer, "	<head>\n");
	add_data(&buffer, "		<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n");
	add_data(&buffer, "		<title>");
	add_data(&buffer, INSTANCE_NAME);
	add_data(&buffer, " - ");
	add_data(&buffer, title);
	add_data(&buffer, "</title>\n");
	add_data(&buffer, "		<style type=\"text/css\">\n");
	add_data(&buffer, "* {\n");
	add_data(&buffer, "	padding: 0;\n");
	add_data(&buffer, "	margin: 0;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "li {\n");
	add_data(&buffer, "	list-style: outside;\n");
	add_data(&buffer, "	margin-left: 1.25em;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "a {\n");
	add_data(&buffer, "	text-decoration: none;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#nav div {\n");
	add_data(&buffer, "	float: left;\n");
	add_data(&buffer, "	margin: 0 0;\n");
	add_data(&buffer, "	padding-left: 0;\n");
	add_data(&buffer, "	padding-right: 20px;\n");
	add_data(&buffer, "	padding-top: 7px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "body {\n");
	add_data(&buffer, "	background-color: #1F4677;\n");
	add_data(&buffer, "	width: 940px;\n");
	add_data(&buffer, "	margin: 5px auto;\n");
	add_data(&buffer, "	font-family: sans-serif;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#nav {\n");
	add_data(&buffer, "	background-color: white;\n");
	add_data(&buffer, "	height: 44px;\n");
	add_data(&buffer, "	padding: 8px;\n");
	add_data(&buffer, "	padding-left: 32px;\n");
	add_data(&buffer, "	font-size: 22px;\n");
	add_data(&buffer, "	font-weight: bold;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#index {\n");
	add_data(&buffer, "	list-style: none;\n");
	add_data(&buffer, "	line-height: normal;\n");
	add_data(&buffer, "	margin: auto 0;\n");
	add_data(&buffer, "	padding-left: 0;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#desc {\n");
	add_data(&buffer, "	background-color: #D2E1F6;\n");
	add_data(&buffer, "	margin: 9px auto;\n");
	add_data(&buffer, "	height: 128px;\n");
	add_data(&buffer, "	padding: 24px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#descinside {\n");
	add_data(&buffer, "	float: left;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#logo {\n");
	add_data(&buffer, "	float: right;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#content {\n");
	add_data(&buffer, "	background-color: #FFFFFF;\n");
	add_data(&buffer, "	margin: -10px auto;\n");
	add_data(&buffer, "	padding: 8px 24px 24px;\n");
	add_data(&buffer, "	min-height: 128px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#pageindex {\n");
	add_data(&buffer, "	background-color: #FFFFFF;\n");
	add_data(&buffer, "	padding-right: 24px;\n");
	add_data(&buffer, "	padding-bottom: 24px;\n");
	add_data(&buffer, "	float: left;\n");
	add_data(&buffer, "	border-right: 4px #1F4677 solid;\n");
	add_data(&buffer, "	width: 150px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#pagecontent {\n");
	add_data(&buffer, "	background-color: #FFFFFF;\n");
	add_data(&buffer, "	width: 670px;\n");
	add_data(&buffer, "	float: right;\n");
	add_data(&buffer, "	padding-left: 24px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#footer {\n");
	add_data(&buffer, "	background-color: #D2E1F6;\n");
	add_data(&buffer, "	padding: 8px 8px 48px;\n");
	add_data(&buffer, "	margin: 8px auto;\n");
	add_data(&buffer, "	font-size: 15px;\n");
	add_data(&buffer, "	height: 32px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, ".fixfloat {\n");
	add_data(&buffer, "	clear: both;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#copyright {\n");
	add_data(&buffer, "	float: right;\n");
	add_data(&buffer, "	font-size: 10px;\n");
	add_data(&buffer, "	margin-top: 16px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#gotop {\n");
	add_data(&buffer, "	position: absolute;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "h2, h3 {\n");
	add_data(&buffer, "	padding-top: 8px;\n");
	add_data(&buffer, "	padding-bottom: 8px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "img {\n");
	add_data(&buffer, "	border: none;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "		</style>\n");
	add_data(&buffer, "	</head>\n");
	add_data(&buffer, "	<body>\n");
	add_data(&buffer, "		<div id=\"nav\">\n");
	add_data(&buffer, "			<div>\n");
	add_data(&buffer, "				<a href=\"");
	add_data(&buffer, INSTANCE_ROOT);
	add_data(&buffer, "/\">Home</a>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div>\n");
	add_data(&buffer, "				<a href=\"");
	add_data(&buffer, INSTANCE_ROOT);
	add_data(&buffer, "/?page=login\">Login</a>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "		<div id=\"desc\">\n");
	add_data(&buffer, "			<div id=\"descinside\">\n");
	add_data(&buffer, "				<h1>");
	add_data(&buffer, title);
	add_data(&buffer, "</h1>\n");
	add_data(&buffer, "				<p>\n");
	add_data(&buffer, desc);
	add_data(&buffer, "				</p>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<img id=\"logo\" src=\"");
	add_data(&buffer, INSTANCE_LOGO);
	add_data(&buffer, "\" height=\"128px\" alt=\"logo\">\n");
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "		<div id=\"content\">\n");
	add_data(&buffer, "			<div id=\"pageindex\">\n");
	add_data(&buffer, "				<h3>Page Menu</h3>\n");
	add_data(&buffer, "				<ul>\n");
	add_data(&buffer, "				</ul>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div id=\"pagecontent\">\n");
	add_data(&buffer, page);
	add_data(&buffer, "				<div class=\"fixfloat\"></div>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "		<div id=\"footer\">\n");
	add_data(&buffer, "			<div id=\"gotop\">\n");
	add_data(&buffer, "				<a href=\"#top\">Top</a>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div id=\"copyright\">\n");
	add_data(&buffer, "				");
	escaped = escape(INSTANCE_ADMIN);
	add_data(&buffer, escaped);
	free(escaped);
	add_data(&buffer, "\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div class=\"fixfloat\"></div>\n");
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "	</body>\n");
	add_data(&buffer, "</html>\n");
}
