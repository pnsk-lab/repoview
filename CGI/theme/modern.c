/* $Id$ */

#include "rv_query.h"

#include "rv_util.h"
#include "rv_version.h"
#include "rv_auth.h"
#include "rv_db.h"
#include "rv_repo.h"
#include "rv_multipart.h"

#include "../../config.h"

#ifdef USE_ENSCRIPT
#include "rv_enscript.h"
#endif

#ifdef USE_AVATAR
#include "rv_avatar.h"
#endif

#ifdef USE_GRAPHICSMAGICK
#include "rv_magick.h"
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char* nocache;
extern char* buffer;
void add_data(char** data, const char* txt);
void render_stuff();

char* title = NULL;
char* desc = NULL;
char* page = NULL;
char* nav = NULL;
char* logo = NULL;
char* grepouser;
extern char* user;

bool invalid_char(char c) {
	if(c >= '0' && c <= '9') return false;
	if(c >= 'a' && c <= 'z') return false;
	if(c >= 'A' && c <= 'Z') return false;
	if(c == '_' && c == '-' && c == '.') return false;
	return true;
}

char* url_escape(const char* input) {
	const char hex[] = "0123456789ABCDEF";
	char* r = malloc(1);
	r[0] = 0;
	char cbuf[2];
	cbuf[1] = 0;
	int i;
	for(i = 0; input[i] != 0; i++) {
		if(input[i] == 0x20 || input[i] == 0x22 || input[i] == 0x25 || input[i] == 0x2d || input[i] == 0x2e || input[i] == 0x3c || input[i] == 0x3e || input[i] == 0x5c || input[i] == 0x5e || input[i] == 0x5f || input[i] == 0x60 || input[i] == 0x7b || input[i] == 0x7c || input[i] == 0x7d || input[i] == 0x7e || input[i] == 0x21 || input[i] == 0x23 || input[i] == 0x24 || input[i] == 0x26 || input[i] == 0x27 || input[i] == 0x28 || input[i] == 0x29 || input[i] == 0x2a || input[i] == 0x2b || input[i] == 0x2c || input[i] == 0x2f || input[i] == 0x3a || input[i] == 0x3b || input[i] == 0x3d || input[i] == 0x3f || input[i] == 0x40 || input[i] == 0x5b || input[i] == 0x5d) {
			add_data(&r, "%");
			cbuf[0] = hex[(input[i] >> 4) & 0xf];
			add_data(&r, cbuf);
			cbuf[0] = hex[input[i] & 0xf];
			add_data(&r, cbuf);
		} else {
			cbuf[0] = input[i];
			add_data(&r, cbuf);
		}
	}
	return r;
}

char* html_escape(const char* input) {
	char* r = malloc(1);
	r[0] = 0;
	char cbuf[2];
	cbuf[1] = 0;
	int i;
	for(i = 0; input[i] != 0; i++) {
		if(input[i] == '<') {
			add_data(&r, "&lt;");
		} else if(input[i] == '>') {
			add_data(&r, "&gt;");
		} else {
			cbuf[0] = input[i];
			add_data(&r, cbuf);
		}
	}
	return r;
}

char* html_escape_nl_to_br(const char* input) {
	char* r = malloc(1);
	r[0] = 0;
	char cbuf[2];
	cbuf[1] = 0;
	int i;
	for(i = 0; input[i] != 0; i++) {
		if(input[i] == '<') {
			add_data(&r, "&lt;");
		} else if(input[i] == '>') {
			add_data(&r, "&gt;");
		} else if(input[i] == '\n') {
			add_data(&r, "<br>");
		} else {
			cbuf[0] = input[i];
			add_data(&r, cbuf);
		}
	}
	return r;
}

void list_repo(const char* name, const char* rev) {
	char* showname = html_escape(name);
	char* urluser = url_escape(user);
	char* urlrepo = url_escape(name);
	add_data(&page, "<tr>");
	add_data(&page, "<td><a href=\"");
	add_data(&page, INSTANCE_ROOT);
	add_data(&page, "/?page=repo&reponame=");
	add_data(&page, urlrepo);
	add_data(&page, "&username=");
	add_data(&page, urluser);
	add_data(&page, "\">");
	add_data(&page, showname);
	add_data(&page, "</a></td>");
	add_data(&page, "<td>");
	add_data(&page, rev);
	add_data(&page, "</td>");
	add_data(&page, "</tr>");
	free(showname);
	free(urluser);
	free(urlrepo);
}

int fcounter = 0;
void list_files(const char* pathname) {
	if(fcounter == 0) {
		add_data(&nav, "<li><a href=\"#filelist\">File List</a></li>\n");
		add_data(&page, "<h2 id=\"filelist\">File List</h2>\n");
		add_data(&page, "<tr style=\"background-color: #D2E1F6;\"><th>Name</th><th>Size</th></tr>\n");
		char* path = rv_get_query("path");
		if(path == NULL) path = "/";
		if(strcmp(path, "/") != 0) {
			char* query = rv_strdup("?page=repo&reponame=");
			char* esc;
			esc = url_escape(rv_get_query("reponame"));
			add_data(&query, esc);
			free(esc);
			add_data(&query, "&username=");
			esc = url_escape(rv_get_query("username"));
			add_data(&query, esc);
			free(esc);
			add_data(&query, "&path=");

			char* urlpath = rv_strdup(path);
			int i;
			int counter = 0;
			int rep = urlpath[strlen(urlpath) - 1] == '/' ? 2 : 1;
			for(i = strlen(urlpath) - 1; i >= 0; i--) {
				char oldc = urlpath[i];
				urlpath[i] = 0;
				if(oldc == '/') {
					counter++;
					if(counter == 2) {
						break;
					}
				}
			}

			if(strlen(urlpath) == 0) {
				free(urlpath);
				urlpath = rv_strdup("/");
			}

			esc = url_escape(urlpath);
			add_data(&query, esc);
			free(esc);

			add_data(&page, "<tr><td><a href=\"");
			add_data(&page, query);
			add_data(&page, "\">../</a></td><td>&lt;DIR&gt;</td></tr>\n");
			fcounter++;
			free(query);
		}
	}
	fcounter++;
	add_data(&page, "<tr style=\"background-color: #");
	if((fcounter % 2) == 0) {
		add_data(&page, "D2E1C0");
	} else {
		add_data(&page, "FFFFFF");
	}
	char* path = rv_get_query("path");
	if(path == NULL) path = "/";
	char* query = rv_strdup("?page=repo&reponame=");
	char* esc;
	esc = url_escape(rv_get_query("reponame"));
	add_data(&query, esc);
	free(esc);
	add_data(&query, "&username=");
	esc = url_escape(rv_get_query("username"));
	add_data(&query, esc);
	free(esc);
	add_data(&query, "&path=");
	char* urlpath = rv_strcat3(path, "/", pathname);
	esc = url_escape(urlpath);
	add_data(&query, esc);
	free(esc);
	char* sz = malloc(128);
	sprintf(sz, "%lld", rv_get_filesize(grepouser, urlpath));
	add_data(&page, "\"><td><a href=\"");
	add_data(&page, query);
	add_data(&page, "\">");
	add_data(&page, pathname);
	add_data(&page, "</a></td>\n");
	add_data(&page, "<td>\n");
	if(strcmp(sz, "-1") != 0) {
		add_data(&page, sz);
	} else {
		add_data(&page, "&lt;DIR&gt;");
	}
	add_data(&page, "</td>\n");
	free(sz);
	add_data(&page, "</tr>\n");
	free(query);
	free(urlpath);
}

void generate_avatar(void) {
	if(user != NULL) {
		char* tmp = rv_strcat3(AVATAR_ROOT, "/", user);
		char* path = rv_strcat(tmp, ".png");
		free(tmp);
		if(access(path, F_OK) != 0) {
			rv_avatar_generate(path, user);
		}
		free(path);
	}
}

void render_page(void) {
	rv_load_query('Q');
	char* query = rv_get_query("page");
	if(query == NULL) query = "welcome";

#ifdef USE_AVATAR
	generate_avatar();
#endif

	if(strcmp(query, "welcome") == 0) {
		title = rv_strdup("Welcome");
		desc = rv_strdup("Welcome to " INSTANCE_NAME ".");
		page = rv_strcat3("Welcome to " INSTANCE_NAME ".<br>This instance is running RepoView version ", rv_get_version(), ".");
#ifdef ALLOW_SIGNUP
	} else if(strcmp(query, "signup") == 0) {
		title = rv_strdup("Signup");
		desc = rv_strdup("You can create your account here.");
		page = rv_strdup("");

		add_data(&page, "<form action=\"");
		add_data(&page, INSTANCE_ROOT);
		add_data(&page, "/?page=sendsignup\" method=\"POST\">\n");
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
		char cbuf[2];
		cbuf[0] = REPO_USER_DELIM;
		cbuf[1] = 0;
		add_data(&page, "Username cannot contain '<code>");
		add_data(&page, cbuf);
		add_data(&page, "</code>'.<br>");
		add_data(&page, "	<input type=\"submit\" value=\"Signup\">\n");
		add_data(&page, "</form>\n");
	} else if(strcmp(query, "sendsignup") == 0) {
		title = rv_strdup("Signup Result");
		page = rv_strdup("");

		rv_load_query('P');
		if(user != NULL) {
			page = rv_strdup("It looks like you are already logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=logout\">log out</a>?\n");
		} else if(rv_get_query("username") == NULL || rv_get_query("password") == NULL) {
			add_data(&page, "Invalid form.\n");
		} else {
			if(rv_has_user(rv_get_query("username"))) {
				add_data(&page, "User already exists.");
			} else {
				if(user != NULL) free(user);
				int i;
				bool reject = false;
				char* name = rv_get_query("username");
				for(i = 0; name[i] != 0; i++) {
					if(name[i] == REPO_USER_DELIM || invalid_char(name[i])) {
						char cbuf[2];
						cbuf[0] = REPO_USER_DELIM;
						cbuf[1] = 0;
						add_data(&page, "Username cannot contain '<code>");
						add_data(&page, cbuf);
						add_data(&page, "</code>'.");
						reject = true;
						break;
					}
				}
				if(!reject) {
					rv_create_user(rv_get_query("username"), rv_get_query("password"));
					user = rv_strdup(rv_get_query("username"));
					add_data(&page, "Welcome.\n");
					rv_save_login(rv_get_query("username"));
				}
			}
		}
#endif
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
		if(user != NULL) {
			page = rv_strdup("It looks like you are already logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=logout\">log out</a>?\n");
		} else if(rv_get_query("username") == NULL || rv_get_query("password") == NULL) {
			add_data(&page, "Invalid form.\n");
		} else {
			if(rv_has_user(rv_get_query("username"))) {
				if(rv_check_password(rv_get_query("username"), rv_get_query("password"))) {
					if(user != NULL) free(user);
					user = rv_strdup(rv_get_query("username"));
					add_data(&page, "Welcome back.\n");
					rv_save_login(rv_get_query("username"));
				} else {
					add_data(&page, "Invalid password.");
				}
			} else {
				add_data(&page, "User does not exist.");
			}
		}
	} else if(strcmp(query, "sendlogout") == 0) {
		title = rv_strdup("Logout Result");
		page = rv_strdup("");
		if(user == NULL) {
			add_data(&page, "You were not logged in.\n");
		} else {
			rv_logout();
			add_data(&page, "Goodbye.\n");
			free(user);
			user = NULL;
		}
#ifdef USE_MYPAGE
	} else if(strcmp(query, "mypage") == 0) {
		title = rv_strdup("My Page");
		desc = rv_strdup("You manage your information here.");
		if(user == NULL) {
			page = rv_strdup("It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else {
			page = rv_strdup("");
			nav = rv_strdup("");
			add_data(&nav, "<li><a href=\"#youricon\">Your Icon</a></li>\n");
			add_data(&nav, "<li><a href=\"#bio\">Bio</a></li>\n");
			add_data(&page, "<form action=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=upload\" method=\"POST\" enctype=\"multipart/form-data\">\n");
			add_data(&page, "	<h2 id=\"youricon\">Your Icon</h2>\n");
			add_data(&page, "	<a href=\"");
			add_data(&page, WWW_AVATAR_ROOT);
			add_data(&page, "/");
			add_data(&page, user);
			add_data(&page, ".png\"><img src=\"");
			add_data(&page, WWW_AVATAR_ROOT);
			add_data(&page, "/");
			add_data(&page, user);
			add_data(&page, ".png");
			add_data(&page, nocache);
			add_data(&page, "\" alt=\"Your Icon\" width=\"50%\"></a><br>");
			add_data(&page, "	<input type=\"file\" name=\"pfp\">\n");
			add_data(&page, "	<h2 id=\"bio\">Bio</h2>\n");
			add_data(&page, "	<textarea name=\"bio\" style=\"width: 100%;resize: none;height: 128px;\">\n");
			char* path = rv_strcat3(BIO_ROOT, "/", user);
			FILE* f = fopen(path, "r");
			if(f != NULL) {
				struct stat s;
				stat(path, &s);
				char* biobuf = malloc(s.st_size + 1);
				fread(biobuf, 1, s.st_size, f);
				biobuf[s.st_size] = 0;

				char* esc = html_escape(biobuf);
				add_data(&page, esc);
				free(esc);

				free(biobuf);
				fclose(f);
			}
			free(path);
			add_data(&page, "</textarea>\n");
			add_data(&page, "	<input type=\"submit\" value=\"Send\">\n");
			add_data(&page, "</form>\n");
		}
#endif
#ifdef USE_AVATAR
	} else if(strcmp(query, "upload") == 0) {
		title = rv_strdup("Uploading My Page Result");
		page = rv_strdup("");
		if(user == NULL) {
			add_data(&page, "It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else {
			struct multipart_entry* entry = rv_get_multipart("pfp");
			if(entry != NULL && entry->length > 0) {
				char* tmp = rv_strcat3(AVATAR_ROOT, "/", user);
				char* path = rv_strcat(tmp, ".tmp");
				char* outpath = rv_strcat(tmp, ".png");
				free(tmp);
				FILE* f = fopen(path, "wb");
				fwrite(entry->data, 1, entry->length, f);
				fclose(f);
				char* reason;
				if(rv_resize_picture(path, outpath, &reason)) {
					add_data(&page, "Uploaded the profile picture successfully.\n");
				} else {
					add_data(&page, "Failed to upload the profile picture.<br><code>\n");
					char* esc = html_escape(reason);
					add_data(&page, esc);
					free(esc);
					add_data(&page, "</code>\n");
					free(reason);
				}
				free(path);
				free(outpath);
			}
			entry = rv_get_multipart("bio");
			if(entry != NULL) {
				char* path = rv_strcat3(BIO_ROOT, "/", user);
				FILE* f = fopen(path, "w");
				fwrite(entry->data, 1, entry->length, f);
				fclose(f);
				free(path);
				add_data(&page, "Uploaded the bio successfully.\n");
			}
		}
#endif
	} else if(strcmp(query, "myrepo") == 0) {
		title = rv_strdup("My Repositories");
		desc = rv_strdup("You manage your repositories here.");
		if(user == NULL) {
			page = rv_strdup("It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else {
			char cbuf[2];
			cbuf[0] = REPO_USER_DELIM;
			cbuf[1] = 0;
			nav = rv_strdup("");
			add_data(&nav, "<li><a href=\"#createrepo\">Create a repository</a></li>\n");
			add_data(&nav, "<li><a href=\"#repolist\">Repository List</a></li>\n");
			page = rv_strdup("");
			add_data(&page, "<h2 id=\"createrepo\">Create a repository</h2>\n");
			add_data(&page, "<form action=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=createrepo\" method=\"POST\">\n");
			add_data(&page, "	<table border=\"0\">\n");
			add_data(&page, "		<tr>\n");
			add_data(&page, "			<th>Repository name</th>\n");
			add_data(&page, "			<td>\n");
			add_data(&page, "				<input name=\"name\">\n");
			add_data(&page, "			</td>\n");
			add_data(&page, "			<td><input type=\"submit\" value=\"Create\"></td>\n");
			add_data(&page, "		</tr>\n");
			add_data(&page, "	</table>\n");
			add_data(&page, "Repository name cannot contain '<code>");
			add_data(&page, cbuf);
			add_data(&page, "</code>'.");
			add_data(&page, "</form>\n");
			add_data(&page, "<h2 id=\"repolist\">Repository List</h2>\n");
			add_data(&page, "<table border=\"0\">\n");
			add_data(&page, "<tr><th>Repository name</th><th>Revision</th></tr>\n");
			rv_repo_list(user, list_repo);
			add_data(&page, "</table>\n");
		}
	} else if(strcmp(query, "createrepo") == 0) {
		title = rv_strdup("Creating Repository Result");
		page = rv_strdup("");

		rv_load_query('P');
		if(user == NULL) {
			page = rv_strdup("It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else if(rv_get_query("name") == NULL) {
			add_data(&page, "Invalid form.\n");
		} else {
			int i;
			bool reject = false;
			char* name = rv_get_query("name");
			for(i = 0; name[i] != 0; i++) {
				if(name[i] == REPO_USER_DELIM || invalid_char(name[i])) {
					char cbuf[2];
					cbuf[0] = REPO_USER_DELIM;
					cbuf[1] = 0;
					add_data(&page, "Repository name cannot contain '<code>");
					add_data(&page, cbuf);
					add_data(&page, "</code>'.");
					reject = true;
					break;
				}
			}
			if(!reject) {
				char* ru = rv_construct_repouser(name, user);
				if(rv_repo_exists(ru)) {
					add_data(&page, "Repository already exists.");
				} else {
					char* esc;
					rv_create_repo(ru);
					add_data(&page, "Repository has been created.<br>\n");
					add_data(&page, "<a href=\"");
					add_data(&page, INSTANCE_ROOT);
					esc = url_escape(name);
					add_data(&page, "/?page=repo&reponame=");
					add_data(&page, esc);
					free(esc);
					esc = url_escape(user);
					add_data(&page, "&username=");
					add_data(&page, esc);
					free(esc);
					add_data(&page, "\">Go to the repository</a>.\n");
				}
				free(ru);
			}
		}
	} else if(strcmp(query, "logout") == 0) {
		title = rv_strdup("Logout");
		desc = rv_strdup("You can log out from your account here.");
		if(user == NULL) {
			page = rv_strdup("It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else {
			page = rv_strdup("");
			add_data(&page, "Are you sure you want to log out?\n");
			add_data(&page, "<form method=\"POST\" action=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=sendlogout\">\n");
			add_data(&page, "	<input type=\"submit\" value=\"Yes\">\n");
			add_data(&page, "</form>\n");
		}
	} else if(strcmp(query, "repo") == 0) {
		title = rv_strdup("Repository");
		desc = rv_strdup("");
		page = rv_strdup("");
		nav = rv_strdup("");
		rv_load_query('Q');
		if(rv_get_query("username") == NULL || rv_get_query("reponame") == NULL) {
			add_data(&page, "Required parameters not set.");
		} else {
			char* ruser = rv_get_query("username");
			char* repo = rv_get_query("reponame");
			char* repouser = rv_construct_repouser(repo, ruser);
			grepouser = repouser;
			if(rv_repo_exists(repouser)) {
				char* showuser = html_escape(ruser);
				char* showrepo = html_escape(repo);
				char* showreadme = rv_get_readme(repouser);
				desc = html_escape_nl_to_br(showreadme);
				add_data(&title, " - ");
				add_data(&title, showuser);
				add_data(&title, "/");
				add_data(&title, showrepo);
				free(showuser);
				free(showrepo);
				free(showreadme);
#ifdef WWW_SVN_ROOT
				add_data(&nav, "<li><a href=\"#repoinfo\">Info</a></li>\n");
				add_data(&page, "<h2 id=\"repoinfo\">Info</h2>\n");
				add_data(&page, "<a href=\"");
				add_data(&page, WWW_SVN_ROOT);
				add_data(&page, "/");
				char* escru = url_escape(repouser);
				add_data(&page, escru);
				free(escru);
				add_data(&page, "\">Raw repository</a>");
#endif

				int isdir;
				char* path = rv_get_query("path");
				if(path == NULL) path = "/";
				fcounter = 0;
				add_data(&page, "<table border=\"0\" style=\"width: 100%;\">");
				bool rej = false;
				if(!rv_get_list(repouser, path, list_files, &isdir)) {
					add_data(&page, "<tr><td>Path not found.</td></tr>\n");
					rej = true;
				}
				add_data(&page, "</table>");
				if(isdir == 0 && !rej) {
					add_data(&nav, "<li><a href=\"#filecontent\">Content</a></li>");
					add_data(&page, "<h2 id=\"filecontent\">Content</h2>\n");
					add_data(&page, "<pre class=\"codeblock\"><code>");
#ifdef USE_ENSCRIPT
					int i;
					char* ext = NULL;
					for(i = strlen(path) - 1; i >= 0; i--) {
						if(path[i] == '.') {
							ext = path + i + 1;
							break;
						}
					}
					char* data = rv_enscript(repouser, path, ext);
					if(data != NULL) {
						add_data(&page, data);
						free(data);
					} else {
						data = rv_read_file(repouser, path);
						if(data != NULL) {
							char* esc = html_escape_nl_to_br(data);
							add_data(&page, esc);
							free(esc);
							free(data);
						} else {
							add_data(&page, "Cannot open the file.\n");
						}
					}
#else
					char* data = rv_read_file(repouser, path);
					if(data != NULL) {
						char* esc = html_escape_nl_to_br(data);
						add_data(&page, esc);
						free(esc);
						free(data);
					} else {
						add_data(&page, "Cannot open the file.\n");
					}
#endif
					add_data(&page, "</code></pre>");
				}
				if(user != NULL && strcmp(user, ruser) == 0) {
					char* esc;
					add_data(&nav, "<li><a href=\"#managerepo\">Manage The Repository</a></li>\n");
					add_data(&page, "<h2 id=\"managerepo\">Manage The Repository</h2>\n");
					add_data(&page, "<form action=\"");
					add_data(&page, INSTANCE_ROOT);
					add_data(&page, "/?page=sendmanrepo&username=");
					esc = url_escape(ruser);
					add_data(&page, esc);
					free(esc);
					add_data(&page, "&reponame=");
					esc = url_escape(repo);
					add_data(&page, esc);
					free(esc);
					add_data(&page, "\" method=\"POST\">\n");
					add_data(&page, "<table border=\"0\" style=\"width: 100%;\">\n");
					add_data(&page, "	<tr>\n");
					add_data(&page, "		<th>README</th>\n");
					add_data(&page, "		<td>\n");
					add_data(&page, "			<textarea name=\"readme\" style=\"width: 100%;resize: none;height: 128px;\">\n");
					char* readme = rv_get_readme(repouser);
					esc = html_escape(readme);
					add_data(&page, esc);
					free(esc);
					free(readme);
					add_data(&page, "</textarea>\n");
					add_data(&page, "		</td>\n");
					add_data(&page, "	</tr>\n");
					add_data(&page, "</table>\n");
					add_data(&page, "<input type=\"submit\" value=\"Send\">\n");
					add_data(&page, "</form>\n");
					add_data(&page, "<a href=\"");
					add_data(&page, INSTANCE_ROOT);
					add_data(&page, "/?page=deleterepo&username=");
					esc = url_escape(ruser);
					add_data(&page, esc);
					free(esc);
					add_data(&page, "&reponame=");
					esc = url_escape(ruser);
					add_data(&page, esc);
					free(esc);
					add_data(&page, "\">\n");
					add_data(&page, "Delete repository\n");
					add_data(&page, "</a>\n");
				}
			} else {
				add_data(&page, "Repository does not exist.\n");
			}
			free(repouser);
		}
	} else if(strcmp(query, "deleterepo") == 0) {
		title = rv_strdup("Delete The Repository");
		page = rv_strdup("");

		rv_load_query('Q');
		if(user == NULL) {
			add_data(&page, "It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else if(rv_get_query("username") == NULL || rv_get_query("reponame") == NULL) {
			add_data(&page, "Invalid Form.\n");
		} else {
			char* esc;
			add_data(&page, "Are you sure you want to delete the repository?\n");
			add_data(&page, "<form method=\"POST\" action=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=senddeleterepo&username=");
			esc = url_escape(rv_get_query("username"));
			add_data(&page, esc);
			free(esc);
			add_data(&page, "&reponame=");
			esc = url_escape(rv_get_query("reponame"));
			add_data(&page, esc);
			free(esc);
			add_data(&page, "\">");
			add_data(&page, "	<input type=\"submit\" value=\"Yes\">\n");
			add_data(&page, "</form>\n");
		}
	} else if(strcmp(query, "senddeleterepo") == 0) {
		title = rv_strdup("Deleting Repository Result");
		page = rv_strdup("");

		rv_load_query('Q');
		if(user == NULL) {
			add_data(&page, "It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else if(rv_get_query("username") == NULL || rv_get_query("reponame") == NULL) {
			add_data(&page, "Invalid Form.\n");
		} else if(strcmp(rv_get_query("username"), user) != 0) {
			add_data(&page, "You are not the owner of the repository.\n");
		} else {
			char* repouser = rv_construct_repouser(rv_get_query("reponame"), rv_get_query("username"));
			if(rv_repo_exists(repouser)) {
				rv_remove_repo(repouser);
				add_data(&page, "Deleted the repository successfully.<br>\n");
			} else {
				add_data(&page, "Repository does not exist.<br>\n");
			}
		}
	} else if(strcmp(query, "person") == 0) {
		title = rv_strdup("Person");
		page = rv_strdup("");

		rv_load_query('Q');
		if(rv_get_query("username") == NULL) {
			add_data(&page, "Invalid Form.\n");
		} else {
			if(rv_has_user(rv_get_query("username"))) {
				add_data(&title, " - ");
				add_data(&title, rv_get_query("username"));
				char* path = rv_strcat3(BIO_ROOT, "/", rv_get_query("username"));
				FILE* f = fopen(path, "r");
				if(f != NULL) {
					struct stat s;
					stat(path, &s);
					char* buf = malloc(s.st_size + 1);
					fread(buf, 1, s.st_size, f);
					buf[s.st_size] = 0;

					desc = html_escape_nl_to_br(buf);

					char* tmp = rv_strcat3(WWW_AVATAR_ROOT, "/", rv_get_query("username"));
					logo = rv_strcat(tmp, ".png");
					free(tmp);

					fclose(f);
				}
				free(path);
			} else {
				add_data(&page, "User does not exist.\n");
			}
		}
	} else if(strcmp(query, "sendmanrepo") == 0) {
		title = rv_strdup("Modifying Repository Result");
		page = rv_strdup("");

		rv_load_query('Q');
		if(user == NULL) {
			add_data(&page, "It looks like you are not logged in.<br>Want to <a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "/?page=login\">log in</a>?\n");
		} else if(rv_get_query("username") == NULL || rv_get_query("reponame") == NULL) {
			add_data(&page, "Invalid Form.\n");
		} else if(strcmp(rv_get_query("username"), user) != 0) {
			add_data(&page, "You are not the owner of the repository.\n");
		} else {
			char* esc;
			rv_load_query('P');
			char* readme = rv_get_query("readme");
			if(readme != NULL) {
				rv_load_query('Q');
				char* name = rv_construct_repouser(rv_get_query("reponame"), rv_get_query("username"));
				rv_set_readme(name, readme);
				free(name);
			}
			rv_load_query('Q');
			add_data(&page, "Modified the repository successfully.<br>\n");
			add_data(&page, "<a href=\"");
			add_data(&page, INSTANCE_ROOT);
			add_data(&page, "?page=repo&username=");
			esc = url_escape(rv_get_query("username"));
			add_data(&page, esc);
			free(esc);
			add_data(&page, "&reponame=");
			esc = url_escape(rv_get_query("reponame"));
			add_data(&page, esc);
			free(esc);
			add_data(&page, "\">Go back to the repository</a>.\n");
		}
	}

	if(title == NULL) title = rv_strdup("");
	if(desc == NULL) desc = rv_strdup("");
	if(page == NULL) page = rv_strdup("");
	if(nav == NULL) nav = rv_strdup("");

#ifdef USE_AVATAR
	generate_avatar();
#endif

	render_stuff();
freeall:
	free(page);
	free(desc);
	free(title);
	free(nav);
	if(logo != NULL) free(logo);
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
	add_data(&buffer, "	padding-right: 25px;\n");
	add_data(&buffer, "	padding-top: 7px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "th,td {\n");
	add_data(&buffer, "	padding: 2px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "body {\n");
	add_data(&buffer, "	background-color: #1F4677;\n");
	add_data(&buffer, "	width: 940px;\n");
	add_data(&buffer, "	margin: 5px auto;\n");
	add_data(&buffer, "	font-family: sans-serif;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#nav {\n");
	add_data(&buffer, "	background-color: white;\n");
	add_data(&buffer, "	background-image: url('");
	add_data(&buffer, INSTANCE_NAVBAR);
	add_data(&buffer, "');\n");
	add_data(&buffer, "	height: 44px;\n");
	add_data(&buffer, "	padding: 8px;\n");
	add_data(&buffer, "	padding-left: 32px;\n");
	add_data(&buffer, "	font-size: 22px;\n");
	add_data(&buffer, "	font-weight: bold;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "pre {\n");
	add_data(&buffer, "	background-color: #dddddd;\n");
	add_data(&buffer, "	border: solid 2px #bbbbbb;\n");
	add_data(&buffer, "	padding: 8px;\n");
	add_data(&buffer, "	overflow: scroll;\n");
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
	add_data(&buffer, "	width: 700px;\n");
	add_data(&buffer, "	overflow-y: scroll;\n");
	add_data(&buffer, "	max-height: 128px;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#logo {\n");
	add_data(&buffer, "	float: right;\n");
	add_data(&buffer, "}\n");
	add_data(&buffer, "#content {\n");
	add_data(&buffer, "	background-color: #FFFFFF;\n");
	add_data(&buffer, "	margin: -10px auto;\n");
	add_data(&buffer, "	padding: 8px 24px 24px;\n");
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
	if(user == NULL) {
		add_data(&buffer, "			<div>\n");
		add_data(&buffer, "				<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=login\">Login</a>\n");
		add_data(&buffer, "			</div>\n");
#ifdef ALLOW_SIGNUP
		add_data(&buffer, "			<div>\n");
		add_data(&buffer, "				<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=signup\">Signup</a>\n");
		add_data(&buffer, "			</div>\n");
#endif
	} else {
#ifdef USE_MYPAGE
		add_data(&buffer, "			<div>\n");
		add_data(&buffer, "				<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=mypage\">My Page</a>\n");
		add_data(&buffer, "			</div>\n");
#endif

		add_data(&buffer, "			<div>\n");
		add_data(&buffer, "				<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=myrepo\">My Repositories</a>\n");
		add_data(&buffer, "			</div>\n");

		add_data(&buffer, "			<div>\n");
		add_data(&buffer, "				<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=logout\">Logout</a>\n");
		add_data(&buffer, "			</div>\n");
	}
	if(user != NULL) {
#ifdef USE_AVATAR
#ifdef USE_MYPAGE
		add_data(&buffer, "<a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=person&username=");
		add_data(&buffer, user);
		add_data(&buffer, "\">");
#endif
		add_data(&buffer, "<img src=\"");
		add_data(&buffer, WWW_AVATAR_ROOT);
		add_data(&buffer, "/");
		add_data(&buffer, user);
		add_data(&buffer, ".png\" alt=\"Your Icon\" style=\"float: right;height: 32px;\">");
#ifdef USE_MYPAGE
		add_data(&buffer, "</a>");
#endif
#endif
		add_data(&buffer, "<div style=\"clear: both;float: right;font-size: 10px;padding-right: 0;font-style: italic;\">You have logged in as <a href=\"");
		add_data(&buffer, INSTANCE_ROOT);
		add_data(&buffer, "/?page=mypage\">");
		add_data(&buffer, user);
		add_data(&buffer, "</a></div>");
	}
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
	if(logo != NULL) {
		add_data(&buffer, logo);
	} else {
		add_data(&buffer, INSTANCE_LOGO);
	}
	add_data(&buffer, "\" height=\"128px\" alt=\"logo\">\n");
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "		<div id=\"content\">\n");
	add_data(&buffer, "			<div id=\"pageindex\">\n");
	add_data(&buffer, "				<h3>Page Menu</h3>\n");
	add_data(&buffer, "				<ul>\n");
	add_data(&buffer, nav);
	add_data(&buffer, "				</ul>\n");
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div id=\"pagecontent\">\n");
	add_data(&buffer, page);
	add_data(&buffer, "			</div>\n");
	add_data(&buffer, "			<div class=\"fixfloat\"></div>\n");
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
#ifdef INSTANCE_BANNERS
	add_data(&buffer, "			<div id=\"banners\" style=\"clear: both;\">\n");
	add_data(&buffer, INSTANCE_BANNERS);
	add_data(&buffer, "			</div>\n");
#else
	add_data(&buffer, "			<div class=\"fixfloat\"></div>\n");
#endif
	add_data(&buffer, "		</div>\n");
	add_data(&buffer, "	</body>\n");
	add_data(&buffer, "</html>\n");
	if(user != NULL) free(user);
}
