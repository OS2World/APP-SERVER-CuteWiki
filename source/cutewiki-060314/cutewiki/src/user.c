/*
 * user.c - all the user related functions
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <crypt.h>

#define PAGE_PRIVATE

#include "cutewiki.h"
#include "cfg.h"
#include "svr.h"
#include "page.h"
#include "page_list.h"
#include "misc.h"
#include "hash.h"
#include "var.h"



static Hash * usertab;



/*
 * Make a copy of the string, but as WikiWord
 */
static void
user_to_wikiword(char * string)
{
    char*	src;
    char*	dst;
    bool        newword;

    /* FIXME: Not Unicode ready! */
    src = string;
    dst = string;
    newword = true;
    while(*src) {
        if (isspace((unsigned char)*src)) {
            newword = true;
            src++;
            continue;
        }

        if (newword)
            *dst++ = toupper((unsigned char)*src++);
        else
            *dst++ = *src++;

        newword = false;
    }
    *dst = '\0';
}



/*
 * set a password for a different user
 */
void
user_set_password(Page * page, const char * password)
{
    free(page->password);
    page->password = strdup( crypt(password, "egal") );
}



/*
 * set own password
 */
void
user_set_own_password(Page * page, const char * password)
{
    user_set_password(page, password);
    svr_set_cookie(server, "cutewiki-user", page->name);
    svr_set_cookie(server, "cutewiki-auth", page->password);
}



/*
 * user_reset_password - reset the user's password to wikiwiki
 *
 * Don't set the cookie, because we may be an other user!
 */
bool
user_reset_password(char * username)
{
    Page * page;

    user_to_wikiword(username);
    if ( (page = pagelist_find_page(username)) ) {
        user_set_password(page, "wikiwiki");
	page_save_meta(page);
	return true;
    }

    return false;
}



/*
 * user_get_password - get an encrypted user password
 */
static char*
user_get_password(Page * page)
{
    return page->password;
}



/*
 * user_check_password - check a not encrypted password against a user page
 */
static bool
user_check_password (Page * page, char * password)
{
    if (page != NULL && page->password != NULL && password != NULL) {
	/* check a not encrypted password */
	if (strcmp(password, page->password) == 0)
	    return true;
    }

    return false;
}



/*
 * user_check_password - check a not encrypted password against a user page
 *
 * The raw password check has an own routine to avoid, that somebody could
 * type in a crypted password in the form.
 */
static bool
user_check_raw_password (Page * page, char * password)
{
    if (page != NULL && page->password != NULL && password != NULL) {
	char* passcrypt;

	passcrypt = crypt(password, "egal");
	if (strcmp(passcrypt, page->password) == 0)
	    return true;
    }

    return false;
}



#if USERID
/*
 * set a user's userid and additionally add it to the user table
 *
 * The latter is done, so that a user can log in with it's
 * short userid or it's full WikiUserName.
 */
void
user_set_userid(Page * page, const char * userid)
{
    /* insert page, if not already there */
    if (hash_find(usertab, userid) == NULL) {
	/* remove the old userid from the user table */
	if (page->userid != NULL)
	    hash_remove(usertab, page->userid);
	free(page->userid);

	/* insert the new page into the user table */
	page->userid = strdup(userid);
	hash_insert(usertab, userid, page);
    }
}


/*
 * Return a password, if available or NULL
 */
char *
user_get_userid(Page * userpage)
{
    return userpage->userid;
}



Page*
user_find_page (const char* userid)
{
#if 0
    fprintf(stderr, "username = %s\n", userid);
    fprintf(stderr, "username len = %d\n", strlen(userid));
#endif
    return (Page *)hash_find(usertab, userid);
}
#endif



/*
 * Get's the actual user of this request
 */
char *
user_get_logname()
{
    return var_get_val(server->variables, "cutewiki-user");
}



/*
 * user_remove - remove a userpage from the hash
 */
bool
user_remove_userid (const char* userid)
{
    return hash_remove(usertab, userid);
}



/*
 * user_logoff - through out user
 */
void
user_logoff(httpd * server)
{
    svr_expire_cookie(server, "cutewiki-user");
    svr_expire_cookie(server, "cutewiki-auth");
}



/*
 * wiki_is_admin - check, if user is an wiki admin
 */
bool
user_is_admin()
{
    Config * cfg;
    char * user;
    char * admin;
    char * key;

    cfg = wiki_get_config();
    user = user_get_logname();

    admin = cfg_first_entry(cfg, "Administration", &key);
    while (admin != NULL) {
	if (!strcmp(admin, user))
	    return true;
	admin = cfg_next_entry(cfg, &key);
    }

    return false;
}



void
user_init()
{
    usertab = hash_new();       /* for fast user lookup */
}

void
user_exit()
{
    /* do nothing for now */
}



/*
 * user_is_authenticated - see, if a user can be authenticated
 */
bool
user_is_authenticated()
{
    Page * page;
    char* username;

    /* check, if a valid authentication cookie is already set*/
    username = var_get_val(server->variables, "cutewiki-user");
    if (username != NULL) {
	page = pagelist_find_page(username);
	if (page != NULL) {
	    char* password = var_get_val(server->variables, "cutewiki-auth");
	    if (user_check_password(page, password)) {
		return true;
	    }
	}
    }

    /* if the passed password is valid, store it in a crypted cookie */
    username = var_get_val(server->variables, "username");
    if (username != NULL) {
	user_to_wikiword(username);
	page = pagelist_find_page(username);
	if (page != NULL) {
	    char* password = var_get_val(server->variables, "password");
	    if (user_check_raw_password(page, password)) {
		password = user_get_password(page);
		svr_set_cookie(server, "cutewiki-user", username);
		svr_set_cookie(server, "cutewiki-auth", password);
		var_set(&server->variables, "cutewiki-user", username);
		return true;
	    }
	}
    }

    /* the user could not be authenticated */
    return false;
}
