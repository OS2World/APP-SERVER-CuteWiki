/*
 * main.c - The cutewiki's main part
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <signal.h>
#include <locale.h>
#include <crypt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include "var.h"
#include "cutewiki.h"
#include "parser.h"
#include "cfg.h"
#include "page.h"
#include "page_list.h"
#include "rss20.h"
#include "user.h"
#include "robot.h"
#include "tar.h"
#include "svr.h"
#include "var.h"
#include "misc.h"
#include "html.h"
#include "request.h"
#include "rcs.h"



struct Wiki {
    char * wikiname;
    Config * cfg;
    char * description;
    char * hostname;
    int    port;
    char * pagedir;
    char * filedir;
    char * imagedir;
    char * wordsdir;

    char * repl_host;
    int    repl_port;
    char * repl_user;
    char * repl_pass;

    char * errorlog;
    char * accesslog;
    FILE * errorfd;
    FILE * accessfd;

    char sysname[64];
    char release[64];
    char machine[64];
    char * username;
    char * password;
    time_t starttime;
    int    calls;               /* number of page calls */
};

struct Wiki * wiki;



/*
 * wiki_get_starttime - get the start time as string
 */
time_t
wiki_get_starttime()
{
    return wiki->starttime;
}



/*
 * wiki_get_calls - See, of a boolean is set in Form
 */
int
wiki_get_calls()
{
    return wiki->calls;
}



/*
 * Returns the name of this Wiki
 */
Config *
wiki_get_config()
{
    return wiki->cfg;
}



/*
 * Returns the name of this Wiki
 */
char *
wiki_get_wikiname()
{
    return wiki->wikiname;
}



/*
 * Returns a short description of this wiki
 */
char *
wiki_get_description()
{
    return wiki->description;
}



/*
 * Returns the network hostname
 */
char *
wiki_get_hostname()
{
    return wiki->hostname;
}



/*
 * Returns the hostname of the host to be replicated
 */
char *
wiki_get_replhost()
{
    return wiki->repl_host;
}



/*
 * Returns the network hostname
 */
char *
wiki_get_machine()
{
    return wiki->machine;
}



/*
 * Returns the network hostname
 */
char *
wiki_get_sysname()
{
    return wiki->sysname;
}



/*
 * Returns the network hostname
 */
char *
wiki_get_release()
{
    return wiki->release;
}



/*
 * Returns the port number
 */
int
wiki_get_port()
{
    return wiki->port;
}



/*
 * Returns the port number
 */
int
wiki_get_replport()
{
    return wiki->repl_port;
}



/*
 * Returns the imagedirectory path
 */
char *
wiki_get_imagedir()
{
    return wiki->imagedir;
}



/*
 * Returns the files directory path
 */
char *
wiki_get_filedir()
{
    return wiki->filedir;
}



/*
 * Returns the imagedirectory path
 */
char *
wiki_get_wordsdir()
{
    /* name of words directory without '/' at the end*/
    return wiki->wordsdir;
}



/*
 * Returns the pages mode as integer
 *
 * RSS mode is never used directly here from a request
 */
int
wiki_get_pagemode()
{
    int retval = MODE_HTML;
    char * mode = var_get_val(server->variables, "mode");

    if (mode) {
        if (!strcmp("htm", mode))
            retval = MODE_HTML;
        else if (!strcmp("reverse", mode))
            retval = MODE_REVERSE;
        else if (!strcmp("edit", mode))
            retval = MODE_EDIT;
        else if (!strcmp("print", mode))
            retval = MODE_PRINT;
        else if (!strcmp("txt", mode))
            retval = MODE_ASCII;
        else if (!strcmp("rtf", mode))
            retval = MODE_RTF;
        else
            retval = MODE_HTML;
    }

    return retval;
}



static char *
wiki_get_pagename (char * dir)
{
    char * name;
    char * ch;

    /* get URI and decode it to ISO-8859-1 */
    name = request_get_uri(server);

    /* get rid of file extension directory and the 2 slashes around */
    name += strlen(dir);

    /* get rid of extension itself */
    ch = name;
    while (*ch && (*ch != '.'))
        ch++;
    *ch = '\0';

    return name;
}



#if 0
static bool
wiki_is_authenticated()
{
    if (var_get_val(server->variables, "action") != NULL) {
        /* we had been logged off, force authenticate the user */
	svr_force_auth(server, wiki_get_wikiname());
	out_write_error("401",  /* not authorized */
			"Wrong Username or password!",
			"Please try again!");
	return false;
    }

    /* authenticate the user */
    if (!user_check_authentication(server)) {
	out_write_error("401",  /* not authorized */
			"Wrong Username or password!",
			"Please try again!");
	return false;
    }
    return true;
}
#endif

/*
 * wiki_check_method
 */
bool
wiki_check_method (int wanted)
{
    int method = request_get_method(server);

    /* check, if we really handle a HTTP_GET method */
    if (method != wanted) {
	if (method == HTTP_GET) {
	    out_write_error("405",  /* wrong method  */
			    "Wrong HTTP method!",
			    "You cann not use a HTTP_GET here!");
	} else if (method == HTTP_POST) {
	    out_write_error("405",  /* wrong method  */
			    "Wrong HTTP method!",
			    "You cann not use a HTTP_POST here!");
	} else {
	    out_write_error("405",  /* wrong method  */
			    "Wrong HTTP method!",
			    "The HTTP method you did use is unknown!");
	}
	return false;
    }

    return true;
}

/*
 * wiki_handle_get - show a wiki page in normal interactive form
 */
static void
wiki_handle_page()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (var_get_val(server->variables, "logoff") != NULL) {
        user_logoff(server);
	html_login_page();
	return;
    }


    name = wiki_get_pagename("/Wiki/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page(name, MODE_NORMAL);
    } else {
        name = "StartPage";
        out_write_page(name, MODE_NORMAL);
        svr_set_response(server, "404");    /* not found */
    }
    wiki->calls++;
}



/*
 * wiki_handle_print - show the page in printable form
 */
static void
wiki_handle_print()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }
    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Print/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page(name, MODE_PRINT);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_text - show the page in ASCII form
 */
static void
wiki_handle_text()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Text/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("SourcePage", MODE_ASCII);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_rtf - deliver a richtext file
 */
static void
wiki_handle_rtf()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Richtext/");
    if (is_wikiword(name)) {
	out_write_page(name, MODE_RTF);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_hist - show the history of a page
 *
 * The revision is already in the request.
 */
static void
wiki_handle_hist()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/History/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("HistoryPage", MODE_NORMAL);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_diff - show the history of a page
 *
 * The revision is already in the request.
 */
static void
wiki_handle_diff()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Diff/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("DiffPage", MODE_NORMAL);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_reverse - show the pages pointing to us
 *
 * To do this we "abuse" the page variable, which now can
 * be used in the ReversePage itself as special field.
 */
static void
wiki_handle_reverse()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Reverse/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("ReversePage", MODE_NORMAL);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



/*
 * wiki_handle_edit - edit the choosen page
 *
 * To do this we "abuse" the page variable, which now can
 * be used in the ReversePage itself as special field.
 */
static void
wiki_handle_edit()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    name = wiki_get_pagename("/Edit/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("EditPage", MODE_EDIT);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}



#if 0
static void
wiki_handle_raw()
{
    char *name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    /* get URI and decode it to ISO-8859-1 */
    name = wiki_get_pagename("/Page/");
    rep_out_page(name);
    wiki->calls++;
}



static void
wiki_handle_meta()
{
    char *name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    /* get URI and decode it to ISO-8859-1 */
    name = wiki_get_pagename("/Meta/");
    rep_out_meta(name);
    wiki->calls++;
}
#endif




static void
wiki_handle_rss()
{
    char *name;

    /* Authentication does not work with most newsreaders */

    if (!wiki_check_method(HTTP_GET))
        return;

    /* get URI and decode it to ISO-8859-1 */
    name = wiki_get_pagename("/Files/");
    rss_handle_feed(name);
}



static void
wiki_handle_tar()
{
    char *name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    /* get URI and decode it to ISO-8859-1 */
    name = wiki_get_pagename("/Files/");
    tar_create_tarfile(name);
}



#if 0
static void
wiki_handle_list()
{
    char *name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_GET))
        return;

    /* get URI and decode it to ISO-8859-1 */
    name = wiki_get_pagename("/Files/");
    rep_create_list(name);
}
#endif



static void
wiki_handle_save()
{
    char * user;
    char * name;
    char * title;
    char * text;
    char * userid;		/* user's shortname */
    char * password;		/* users password */
    char * group;		/* name of a group page */
    char * type;		/* type of a group page */
    char * topic;		/* topic of a page */
    int seqno;
    bool saved;
    bool priv;
    bool hidden;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_POST))
        return;

    /* If page was submitted, then try to save it */
    name = var_get_val(server->variables, "page");
    if (name == NULL) {
	out_write_error("406",  /* not acceptable */
			"There is no page name in the POST data!",
			"How can you save a not named page?");
	return;
    }

    if (!is_wikiword(name)) {
	out_write_error("406",  /* not acceptable */
			"This is no valid page title!",
			"The page title must be a WikiWord!");
	return;
    }

    /* if the text was empty, then delete the page */
    text = var_get_val(server->variables, "text");
    if (text == NULL || strlen(text) == 0) {
	if (page_del(pagelist_find_page(name))) {
	    out_write_page("DeletePage", MODE_HTML);
	}
	return;
    }

    user = user_get_logname();
    title = var_get_val(server->variables, "title");
    group = var_get_val(server->variables, "group");
    type = var_get_val(server->variables, "pagetype");
    topic = var_get_val(server->variables, "topic");
    password = var_get_val(server->variables, "password");
    userid = var_get_val(server->variables, "userid");

    seqno = var_get_int(server->variables, "seqno");
    priv = var_get_bool(server->variables, "private");
    hidden = var_get_bool(server->variables, "hidden");

    /* now really change the page, given all the data */
    saved = page_edit(name, title, text, topic, user, userid, password,
		      group, type, seqno, priv, hidden);

    if (saved)
	out_write_page(name, MODE_HTML);
    else
	out_write_error("500",  /* internal error */
			"Could not save the page!",
			"Maybe, somebody other has changed it in the meantime?");
}



/*
 * wiki_handle_pwreset - user get's password reset
 */
static void
wiki_handle_pwreset()
{
    char * username;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_POST))
        return;

    /* Handle password reset */
    username = var_get_val(server->variables, "pwreset");
    if (username != NULL && user_reset_password(username))
	out_write_page(username, MODE_HTML);
    else
	out_write_page("StartPage", MODE_HTML);
    wiki->calls++;
}

static void
wiki_handle_search()
{
    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_POST))
        return;

    out_write_page("SearchPage", MODE_NORMAL);
    wiki->calls++;
}



/*
 * wiki_handle_support - handle a support question
 */
static void
wiki_handle_support()
{
    char* question;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    if (!wiki_check_method(HTTP_POST))
        return;

    /* Handle support bot */
    question = var_get_val(server->variables, "question");
    if (question != NULL)
	out_write_page("SupportPage", MODE_HTML);
    else
	out_write_page("StartPage", MODE_HTML);
    wiki->calls++;
}



/*
 * wiki_handle_filter - switch off a set filter
 */
static void
wiki_handle_filter()
{
    char* name;

    if (!user_is_authenticated()) {
	html_login_page();
	return;
    }

    /* unset the cookie and cutewiki-category variable */
    svr_expire_cookie(server, "cutewiki-category");
    var_del(&server->variables, "cutewiki-category");

    name = wiki_get_pagename("/FilterOff/");
    if (is_wikiword(name)) {
	var_set(&server->variables, "page", name);
	out_write_page("CategoryPage", MODE_NORMAL);
    } else {
	svr_set_response(server, "404");    /* not found */
	out_write_page("StartPage", MODE_NORMAL);
    }
    wiki->calls++;
}






static void
wiki_init(char * wikiname)
{
    struct utsname name;

    /* First initialize general settings */
    wiki = malloc(sizeof(struct Wiki));
    wiki->starttime = time(NULL);
    wiki->wikiname = wikiname;
    wiki->cfg = cfg_read_mycfg(wikiname);
    if (wiki->cfg == NULL) {
        fprintf(stderr, "Error: Can not read config file for wiki of name %s!\n", wikiname);
        exit (1);
    }

    /* Read in essential settings, exit if not set. */
    wiki->calls = 0;
    wiki->description = cfg_check_str(wiki->cfg, "General", "description", true);
    wiki->hostname = cfg_check_str(wiki->cfg, "General", "hostname", true);
    wiki->port = cfg_check_int(wiki->cfg, "General","port", 8080, true);
    wiki->filedir = cfg_check_str(wiki->cfg, "Files", "filedir", true);
    wiki->imagedir = cfg_check_str(wiki->cfg, "Files", "imagedir", true);
    wiki->pagedir = cfg_check_str(wiki->cfg, "Files", "pagedir", true);
    wiki->accesslog = cfg_check_str(wiki->cfg, "Files", "accesslog", true);
    wiki->errorlog = cfg_check_str(wiki->cfg, "Files", "errorlog", true);

#if 0
    wiki->wordsdir = cfg_check_str(wiki->cfg, "Files", "wordsdir", true);
#endif
#if 0
    /* Iitialize replication settings */
    wiki->repl_host = cfg_check_str(wiki->cfg, "Replication", "hostname", NULL);
    wiki->repl_port = cfg_check_int(wiki->cfg, "Replication","port", 1);
    if (wiki->repl_port == 1) {
        fprintf(stderr, "Error: No Port specified in [Replication] section. Exiting!\n");
        exit (1);
    }
#endif

    /* get some platform information from the OS */
    uname(&name);
    strncpy(wiki->sysname, name.sysname, 64);
    strncpy(wiki->release, name.release, 64);
    strncpy(wiki->machine, name.machine, 64);

    /* Create a server instance and set it up */
    server = svr_new(NULL, wiki->port);
    if (server == NULL) {
        perror("Can't create server");
        exit(1);
    }
    svr_set_filebase(server, wiki->pagedir);
    svr_set_errorlog(server, stderr);
    svr_set_accesslog(server, stdout);

    /* set stderr und stdout */
    wiki->errorfd = fopen(wiki->errorlog, "a+");
    if (wiki->errorfd == NULL)
        fprintf(stderr, "Error: Can not write Errorlog at %s!\n", wiki->errorlog);
    else
	svr_set_errorlog(server, wiki->errorfd);
    wiki->accessfd = fopen(wiki->accesslog, "a+");
    if (wiki->accessfd == NULL)
	fprintf(stderr, "Error: Can not write Accesslog at %s!\n", wiki->accesslog);
    else
	svr_set_accesslog(server, wiki->accessfd);

    /* handle the different content */
    svr_register_dirhandler(server,"/", NULL, wiki_handle_page);
    svr_register_dirhandler(server,"/Wiki", NULL, wiki_handle_page);
    svr_register_dirhandler(server,"/Edit", NULL, wiki_handle_edit);
    svr_register_dirhandler(server,"/Reverse", NULL, wiki_handle_reverse);
    svr_register_dirhandler(server,"/Print", NULL, wiki_handle_print);
    svr_register_dirhandler(server,"/Text", NULL, wiki_handle_text);
    svr_register_dirhandler(server,"/History", NULL, wiki_handle_hist);
    svr_register_dirhandler(server,"/Diff", NULL, wiki_handle_diff);
    svr_register_dirhandler(server,"/Richtext", NULL, wiki_handle_rtf);

    /* handle the http posting of information */
    svr_register_dirhandler(server,"/Save", NULL, wiki_handle_save);
    svr_register_dirhandler(server,"/Search", NULL, wiki_handle_search);
    svr_register_dirhandler(server,"/Password", NULL, wiki_handle_pwreset);
    svr_register_dirhandler(server,"/Support", NULL, wiki_handle_support);

    /* handle special cases */
    svr_register_dirhandler(server,"/FilterOff", NULL, wiki_handle_filter);
    svr_register_dirhandler(server,"/Backup", NULL, wiki_handle_tar);
    svr_register_dir(server,"/Files", NULL, wiki->filedir);
    svr_register_dir(server,"/Images", NULL, wiki->imagedir);
    svr_register_filehandler(server,"/Files", "changes.rss", HTTP_FALSE, NULL, wiki_handle_rss);

#if 0
    svr_register_filehandler(server,"/Files", "allpages.tar", HTTP_FALSE, NULL, wiki_handle_tar);
    svr_register_filehandler(server,"/Files", "allpages.txt", HTTP_FALSE, NULL, wiki_handle_list);
    svr_register_dirhandler(server,"/Page", NULL, wiki_handle_raw);
    svr_register_dirhandler(server,"/Meta", NULL, wiki_handle_meta);
#endif

    out = &htm;                /* set to normal HTML output */

    /* Avoid of crash, if too many requests are pending */
    signal(SIGPIPE, SIG_IGN);

}

static void
wiki_loop()
{
    struct timeval timeout;
    int    result;

    /* Go into our service loop */
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    while(1) {
        result = svr_get_connection(server, &timeout);
        if (result == 0) {
            continue;
        }
        if (result < 0) {
            printf("Error in mainloop... \n");
            continue;
        }
        if(svr_read_request(server) < 0) {
	    printf("Error in mainloop at read_request... \n");
            svr_end_request(server);
            continue;
        }

        svr_process_request(server);
        svr_end_request(server);
    }
}

static void
wiki_exit()
{
    /* Now shutdown everything */
    if (wiki->errorfd) {
        fclose(wiki->errorfd);
    }

    if (wiki->accessfd) {
        fclose(wiki->accessfd);
    }
    svr_del(server);
    cfg_del(wiki->cfg);

}



int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"usage: cutewiki <wikiname> \n");
        exit(1);
    }

    /* Try to set different non-UTF-8 locale */
    if (!setlocale(LC_ALL, "")) {
	fprintf(stderr, "Error: Setting locale failed!\n");
	exit(1);
    }

    //svr_init();
    wiki_init(argv[1]);
    user_init();
    rcs_init();
    pagelist_init(wiki->pagedir);
    fprintf(stderr, "Info:  CuteWiki started with configuration '%s'.\n", wiki->wikiname);
    wiki_loop();
    pagelist_exit();
    user_exit();
    wiki_exit();
    svr_exit();

    return 0;
}

