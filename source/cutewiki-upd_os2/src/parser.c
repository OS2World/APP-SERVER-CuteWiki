/*
 * parser.c - The parser for cutewiki's ASCII pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cutewiki.h"
#include "page.h"
#include "page_list.h"
#include "create.h"
#include "robot.h"
#include "user.h"
#include "var.h"
#include "parser.h"
#include "misc.h"
#include "rcs.h"



#define SHOW_NOTHING 0
#define SHOW_OWNER   1
#define SHOW_DATE    2

/*
 * LineFmt - line formatting data
 */
typedef struct LineFmt LineFmt;
struct LineFmt
{
    bool italic;
    bool bold;
    bool number;
};



/*
 * ParseState - paragraph formatting data
 */
typedef struct ParseState ParseState;
struct ParseState
{
    int		indent;		/* amount of indent, 0 to 3 */
    char	indents[4];	/* indent data; o = ol; u = ul */
    int		quote;		/* quote depth */
    bool	pre;		/* preformatted flag */
    bool	para;		/* paragraph flag */
    bool	ruler;		/* line is a ruler */
    int         head;		/* heading level */
    int         table;		/* 1=table, 2=tabletop */
    int         cells;		/* count of cells in a table row */
    bool        comment;        /* skip this line, it's a comment */
};



/* prototypes */
static void	do_string(char*, ParseState *);



/* The choosen output option */
Output * out;



/*
 * isurl - get valid URL characters
 */
static bool
isurl(char c)
{
    // leaf characters
    if (isalnum((unsigned char)c)
	|| (c == '+') || (c == '-') ||(c == '_')
	// URL seperators
	|| (c == ':') || (c == '/') || (c == '.') || (c == ',') || (c == '#')
	// CGI access characters
	|| (c == '+') || (c == '%') || (c == '?') || (c == '&') || (c == '=')
	// email characters
	|| (c == '@')
	// other allowed characters
	|| (c == '(') || (c == ')') || (c == ';') || (c == '~') )
	return true;

    return false;
}



/*
 * dup_string - duplicate a string
 *
 * end is pointing to the first char after the wanted string
 */
static char*
dup_string(const char* start, const char* end)
{
    char* string;
    int len = end - start;

    string = malloc(len + 1);
    memcpy(string, start, len);
    string[len] = '\0';

    return string;
}



/*
 * get_space - eat some spaces
 */
static void
get_space(char** line)
{
    char* ch;

    ch = *line;
    while (*ch == ' ' || *ch == '\t')
        ch++;

    *line = ch;
}



/*
 * get_alnum - get an alphanumeric string
 */
char*
get_alnum(char** line)
{
    char* start;
    char* end;

    start = *line;
    end = *line;
    while (isalnum((unsigned char)*end) || *end == '_')
        end++;
    *line = end;

    return dup_string(start, end);
}



#if 0
/*
 * get_number - get a number
 */
static char*
get_number(char** line)
{
    char* start;
    char* end;

    start = *line;
    end = start;
    while (isdigit((unsigned char)*end))
        end++;
    *line = end;

    return dup_string(start, end);
}
#endif

/*
 * get_line - get line terminated eventually by <CR> and <LF>
 */
static char*
get_line(char** text)
{
    char* start;
    char* end;
    char* line;

    start = end = *text;
    while (*end && *end != '\r' && *end != '\n')
	end++;
    line = dup_string(start, end);

    /* skip the line ending codes, if given */
    if (*end == '\r')
	end++;
    if (*end == '\n')
	end++;
    *text = end;

    return line;
}

/*
 * get_url - get an URL
 */
static char*
get_url(char** line)
{
    char* start;
    char* end;

    start = end = *line;
    while (isurl(*end))
        end++;
    *line = end;

    return dup_string(start, end);
}



/*
 * get_cell - get the next cell of a table
 */
static char*
get_cell(char** line)
{
    char* start;
    char* end;

    start = end = *line;
    while (*end && *end != '|')
        end++;
    *line = end;

    return dup_string(start, end);
}



/*
 * get_square - get a special field's
 */
static char*
get_square(char** line)
{
    char* start;
    char* end;

    start = end = *line;
    while (*end && *end != ']')
	end++;
    *line = end;     /* step over bracket or newline */

    return dup_string(start, end);
}



/*
 * get_shortdate - get'sdate in standard format
 */
static char*
get_shortdate (char * datestring)
{
    time_t acttime = time(NULL);
    strftime(datestring, MAX_DATELEN-1, "%y%m%d", localtime(&acttime));
    datestring[MAX_DATELEN-1] = '\0';

    return datestring;
}



static bool
is_cell(char* lp)
{
    while (*lp && *lp != '|')
        lp++;
    if (*lp == '|')
        return true;
    else
        return false;
}



static bool
is_url(char* lp)
{
    if (!strcmp(lp, "http")) {
        return true;
    }
    else if (!strcmp(lp, "mailto")) {
        return true;
    }
    else if (!strcmp(lp, "ftp")) {
        return true;
    }
    else if (!strcmp(lp, "file")) {
        return true;
    }
    else if (!strcmp(lp, "news")) {
        return true;
    }
    else if (!strcmp(lp, "https")) {
        return true;
    }
    else if (!strcmp(lp, "gopher")) {
        return true;
    }
    else if (!strcmp(lp, "telnet")) {
        return true;
    }
    else if (!strcmp(lp, "mms")) {
        return true;
    }

    return false;
}



#if 0
static bool
is_footnote(char* lp)
{
    if (!isdigit((unsigned char)*lp))
	return false;
    lp++;

    while (isdigit((unsigned char)*lp))
	lp++;

    if (*lp != ' ')
        return false;

    return true;
}
#endif


static bool
is_numbercell(char* lp)
{
    get_space(&lp);
    if (*lp == '-') {
        lp++;
    }
    while (isdigit((unsigned char)*lp) || *lp == ',' || *lp == '.') {
        lp++;
    }
    get_space(&lp);
    if (*lp != '\0')
        return false;
    else
        return true;
}


/*
 * count_cells - Count, how many cells a table has in a row
 */
static void
count_cells(char* lp, ParseState * state)
{
    char* lp2;

    state->cells = 0;
    lp2 = lp+1;
    while (*lp2) {
        if (*lp2 == '|')
            state->cells++;
        lp2++;
    }
}



static void
do_cell(char** line, ParseState* pfmt)
{
    char* cell;
    char* lp;

    lp = *line;

    cell = get_cell(&lp);
    if (is_numbercell(cell)) {
        out->TableNumberBegin();
        do_string(cell, pfmt);      /* recursive call for markup in Cell */
        out->TableNumberEnd();
    }
    else {
        out->TableCellBegin();
        do_string(cell, pfmt);      /* recursive call for markup in Cell */
        out->TableCellEnd();
    }
    free(cell);

    *line = lp;
}



static void
do_index()
{
    Page**	list;
    char section;
    bool first_section;
    int i;

    section = ' ';      /* divide the list by character sections */
    first_section = true;
    list = pagelist_alpha_sorted();

    for (i = 0; list[i] != NULL; i++) {
	char buf[HTTP_MAX_LEN];
	char time[MAX_TIMELEN];
        char cap;       /* first capital letter of a wikiword */

        if (!page_is_seen(list[i]))
            continue;

        /* see, if we begin a new capital letter */
        cap = toupper(*page_get_title(list[i]));
        if (cap != section) {
            if (first_section == false) {
		/* end old  section */
                out->ListEnd();
                out->ParaEnd();
            }

	    /* begin a new section */
            out->ParaBegin();
            out->HeadingBegin(1);
            out->Putc(cap);
            out->HeadingEnd(1);
            out->ParaEnd();
            out->ParaBegin();
            out->ListBegin();

	    section = cap;
            first_section = false;
        }
        out->ListItemBegin();
        out->LineBegin();
	out->InternalLink(page_get_name(list[i]),
			  page_get_title(list[i]),
			  page_get_type(list[i])
			 );
	page_get_timestring(list[i], time);
	sprintf(buf, "   -   %s, %s", time, page_get_ownername(list[i]) );
        out->Puts(buf);
        out->LineEnd();
        out->ListItemEnd();
    }
    out->ListEnd();
    out->ParaEnd();
    free(list);
}



/*
 * do_changes ()
 */
static void
do_changes ()
{
    Page**	list;
    int	ndays;
    bool started;
    char buf [HTTP_MAX_LEN];
    char olddate[MAX_DATELEN];
    char newdate[MAX_DATELEN];
    int i;

    olddate[0] = '\0';
    list = pagelist_time_sorted();
    ndays = 21;
    started = false;
    for (i = 0; list[i] != NULL; i++) {
        if (page_is_hidden(list[i]))
            if (strcmp(page_get_owner(list[i]), user_get_logname()))
                continue;

        page_get_datestring(list[i], newdate);
        if (strcmp(olddate, newdate) != 0 ) {
            ndays--;
            if (!ndays)
                break;

            if (started) {
                out->ListEnd();
                out->ParaEnd();
            }
            started = true;
            out->ParaBegin();
            out->HeadingBegin(1);
            out->Puts(newdate);
            out->HeadingEnd(1);
            out->ParaEnd();
            out->ParaBegin();
            out->ListBegin();

            strncpy(olddate, newdate, MAX_DATELEN);
        }
        out->ListItemBegin();
        out->LineBegin();
	out->InternalLink(page_get_name(list[i]),
			  page_get_title(list[i]),
			  page_get_type(list[i])
			 );
        sprintf(buf, "   -   %s", page_get_owner(list[i]) );
        out->Puts(buf);
        out->LineEnd();
        out->ListItemEnd();
    }
    out->ListEnd();
    out->ParaEnd();

    free(list);
}






/*
 * show_editform - Make editing of a a page possible
 *
 * This all get's expandet just from a [EditForm] Tag in our
 * EditPage. This page must always be available!
 */
static void
show_editform(Page * page)
{
    /* get the pages data in */
    char * name = page_get_name(page);
    char * title = page_get_title(page);
    char * topic = page_get_topic(page);
    char * content = page_get_text(page);

    bool hidden = page_is_hidden(page);
    int seqno = page_get_seqno(page);
    bool private = page_is_private(page);
    Pagetype pagetype = page_get_type(page);
    char * groupname = page_get_groupname(page);
//    char * userid = user_get_userid(page);

    /* now really print all the form */
    svr_printf(server,
	       "<form method=\"post\" accept-charset=\"UTF-8\" "
	       "action=\"/Save/Result\">\n");

#if GERMAN
    svr_printf(server,
	       "  <b>Titel:</b> <input type=\"text\" size=\"30\" "
	       "name=\"title\" value=\"%s\">\n", title);
    svr_printf(server,
	       "  <b>Thema:</b> <input type=\"text\" size=\"30\" "
	       "name=\"topic\" value=\"%s\">\n", topic ? topic : "");
#else
    svr_printf(server,
	       "  <b>Title:</b> <input type=\"text\" size=\"30\" "
	       "name=\"title\" value=\"%s\">\n", title);
    svr_printf(server,
	       "  <b>Topic:</b> <input type=\"text\" size=\"30\" "
	       "name=\"topic\" value=\"%s\">\n", topic ? topic : "");
#endif

    /* If this is the actual user's homepage, print userid, password */
    if (!strcmp(name, user_get_logname())) {
#if 0
	svr_printf(server,
		   "<b>Userid:</b> <input type=\"text\" size=\"10\" "
		   "name=\"userid\" value=\"%s\">\n", userid ? userid : "");
#endif
	svr_printf(server,
		   "<b>Password:</b> <input type=\"password\" "
		   "size=\"15\" name=\"password\" value=\"\">\n");
    }

    /* If this is a normal page, let's change it possibly */
    if (pagetype == PT_NORMAL) {
	svr_printf(server, "<span class=\"right\">");
	svr_printf(server, "<b>Pagetype:</b> ");
        svr_printf(server, "<select name=\"pagetype\" size=\"1\">\n");
        svr_printf(server,
                   "<option selected>Normal</option>\n"
                   "<option>Homepage</option>\n"
                   "<option>Grouppage</option>\n"
                   "<option>Category</option>\n");
        svr_printf(server, "</select>\n");
	svr_printf(server, "</span>");
    }

    /* print the real page's content */
    svr_printf(server,
	       "<br><br><textarea class=\"wikisource\" name=\"text\" "
	       "cols=\"116\" rows=\"21\" >\n");
    xml_puts(content);
    svr_printf(server, "</textarea><br><br>\n");


    /* We can set a group, if it is no homepage */
    if (pagetype != PT_USER) {
#if GERMAN
	svr_printf(server,
		   "<b>Gruppe:</b> <input type=\"text\" size=\"30\" "
		   "name=\"group\" value=\"%s\">\n",
		   (groupname) ? groupname : "" );
#else
	svr_printf(server,
		   "<b>Group:</b> <input type=\"text\" size=\"30\" "
		   "name=\"group\" value=\"%s\">\n",
		   (groupname) ? groupname : "" );
#endif
    }
    svr_printf(server, "   \n");

    /* display the other attributes */
#if GERMAN
    svr_printf(server,
	       "<input type=\"checkbox\" name=\"private\" value=\"yes\" %s> "
	       "<b>Schreibgeschützt</b>\n", private ? "checked" : "");
    svr_printf(server,
	       "<input type=\"checkbox\" name=\"hidden\" value=\"yes\" %s> "
	       "<b>Versteckt</b>\n", hidden ? "checked" : "");
#else
    svr_printf(server,
	       "<input type=\"checkbox\" name=\"private\" value=\"yes\" %s> "
	       "<b>Read Only</b>\n", private ? "checked" : "");
    svr_printf(server,
	       "<input type=\"checkbox\" name=\"hidden\" value=\"yes\" %s> "
	       "<b>Hidden</b>\n", hidden ? "checked" : "");
#endif

    svr_printf(server, "<span class=\"right\">");
    svr_printf(server,
	       "<input class=\"content\"type=\"submit\" "
	       "value=\" Save \">\n");
    svr_printf(server,
	       "<input class=\"content\" type=\"reset\" "
	       "value=\" Reset \"><br>\n");
    svr_printf(server, "</span>");

    svr_printf(server,
	       "<input type=\"hidden\" name=\"page\" "
	       "value=\"%s\">\n", name);
    svr_printf(server,
	       "<input type=\"hidden\" name=\"seqno\" "
	       "value=\"%d\">\n", seqno);

    svr_printf(server, "</form>\n");
}



/*
 * show_sourceform - Show the text source of the page
 */
static void
show_sourceform(Page * page)
{
    /* get the pages data in */
    char * content = page_get_text(page);

    /* now really print all the form */
    svr_puts(server,
	     "<form method=\"post\" accept-charset=\"UTF-8\" "
	     "action=\"/Save/Result\">\n");

    /* print the real page's content */
    svr_puts(server,
	       "<textarea class=\"wikisource\" name=\"text\" "
	       "cols=\"116\" rows=\"21\" readonly>\n");
    xml_puts(content);
    svr_puts(server, "</textarea><br><br>\n");

    svr_puts(server, "</form>\n");
}



static void
show_existent_page (Page * page)
{
    char * user;
    bool loaded;

    /* check, if it is allowed to edit */
    if (!page_is_writable(page)) {
	svr_set_response(server, "403");    /* forbidden */
#if GERMAN
	out->Puts("Sie dürfen diese Seite nicht ändern!");
#else
	out->Puts("You are not allowed to change this page!");
#endif
	return;
    }

    /* maybe somebody other does edit the page? */
    user = user_get_logname();
    if (page_is_edited(page)) {
	char * editor = page_get_editor(page);

        /* if we are the one, editing is ok */
	if (strcmp(editor, user) != 0) {
#if GERMAN
	    out->Puts("Hallo, ");
	    out->Puts(user);
	    out->Puts("! Diese Seite wird gerade von ");
	    out->Puts(editor);
	    out->Puts(" editiert! Bitte einen Augenblick Geduld...");
#else
	    out->Puts("Hello, ");
	    out->Puts(user);
	    out->Puts("! In this moment the page gets edited by ");
	    out->Puts(editor);
	    out->Puts(", please wait a moment and try it later...");
#endif
	    return;
	}
    }

    /* now really edit the page */
    page_load_text(page, &loaded);
    show_editform(page);
    page_unload_text(page, loaded);

    /* Now the timer is running for the others... */
    page_set_editor(page, user);
}



static void
show_new_page (const char * name)
{
    /* show a new page */
    if (is_wikiword(name)) {
	Page * page = page_new(name, PT_NORMAL);
	if (page) {
	    show_editform(page);
	    page_del(page);
	}
    } else {
	svr_set_response(server, "500");
#if GERMAN
	out->Puts("Dies ist kein erlaubter Seitenname!");
#else
	out->Puts("This page name is not allowed!");
#endif
    }
}



/*
 * do_editform - write an edit form, if possible
 */
static void
do_editform()
{
    char* name;

    name = var_get_val(server->variables, "page");
    if (name != NULL) {
	Page * page;

	page = pagelist_find_page(name);
	if (page) {
            show_existent_page(page);
	}
	else {
	    show_new_page(name);
	}
    }
    else
	out->Puts("[EditPage]");
}



/*
 * do_sourceform - write the page's text as form
 */
static void
do_sourceform()
{
    char* name;

    name = var_get_val(server->variables, "page");
    if (name != NULL) {
	Page * page;

	page = pagelist_find_page(name);
	if (page) {
	    bool loaded;

	    /* now really edit the page */
	    page_load_text(page, &loaded);
	    show_sourceform(page);
	    page_unload_text(page, loaded);
	}
    }
    else
	out->Puts("[EditPage]");
}



/*
 * do_errormsg - write the given error message
 */
static void
do_errormsg()
{
    char* msg;

    msg = var_get_val(server->variables, "errormsg");
    if (msg != NULL)
	out->Puts(msg);
    else
	out->Puts("[ErrorMessage]");
}



/*
 * do_errordsc - write an error description
 */
static void
do_errordsc()
{
    char* dsc;

    dsc = var_get_val(server->variables, "errordsc");
    if (dsc != NULL)
	out->Puts(dsc);
    else
	out->Puts("[ErrorDescription]");
}



static void
do_list(Page ** list, int info)
{
    int i;

    /* If a search result is given, show it... */
    if (list == NULL) {
	/* no page was found */
#if GERMAN
	out->Puts("Kein Ergebnis!");
#else
	out->Puts("No Result!");
#endif
    }
    else {
	/* print list of pages */
        out->ListBegin();
        for (i = 0; i < pagelist_get_count(); i++) {
	    char time[MAX_TIMELEN];

            if (!list[i])
                break;

            if (!page_is_seen(list[i]))
                    continue;

            /* show the line with or without extra info */
            out->ListItemBegin();
	    out->InternalLink(page_get_name(list[i]),
			      page_get_title(list[i]),
			      page_get_type(list[i]));
	    if (info) {
		out->Puts("   -   ");
	    }

	    if (info & SHOW_DATE) {
		out->Puts(page_get_timestring(list[i], time));
	    }
	    if (info & SHOW_OWNER) {
		out->Puts(", ");
		out->Puts(page_get_ownername(list[i]));
	    }
            out->ListItemEnd();
        }
        out->ListEnd();
	free(list);
    }
}



/*
 * do_pagelist - prints a date sorted list of all pages
 */
static void
do_pagelist()
{
    do_list(pagelist_time_sorted(), SHOW_DATE|SHOW_OWNER);
}



static void
do_reverselist()
{
    char* name;

    name = var_get_val(server->variables, "page");
    if (name != NULL)
	do_list(pagelist_of_reverse_links(name), SHOW_DATE|SHOW_OWNER);
    else
	out->Puts("[ReverseList]");
}



static void
do_searchlist()
{
    char* full;
    char* criterion;
    char* filter;

    /* Handle search form data */
    full = var_get_val(server->variables, "fullsearch");
    criterion = var_get_val(server->variables, "cutewiki-search");
    filter = var_get_val(server->variables, "category");

    /* if search string is empty, write error */
    if (criterion != NULL) {
	if (full != NULL) {
	    do_list(pagelist_search_full(criterion, filter),
		    SHOW_DATE|SHOW_OWNER);
	}
	else {
	    do_list(pagelist_search_title(criterion, filter),
		    SHOW_DATE|SHOW_OWNER);
	}
    }
    else {
#if GERMAN
	out->Puts("Kein Suchkriterium gegeben!");
#else
	out->Puts("No search criteria given!");
#endif
    }
}



/*
 * do_userlist - prints a list of all registered users
 */
static void
do_userlist()
{
    do_list(pagelist_get_users(), SHOW_DATE);
}



/*
 * do_userlist - prints a list of all registered users
 */
static void
do_grouplist()
{
    do_list(pagelist_get_groups(), SHOW_DATE|SHOW_OWNER);
}



/*
 * do_categorylist - prints a list of all tagged categories
 */
static void
do_categorylist()
{
    do_list(pagelist_get_categories(), SHOW_DATE|SHOW_OWNER);
}



/*
 * do_user - show the user, it it exists or not
 */
static void
do_user ()
{
    Page * page;
    char * username;

    username = user_get_logname();
    page = pagelist_find_page(username);
    if (page) {
	/* The page exists! */
	if (!page_is_seen(page))
	    out->Puts(page_get_title(page));
	else
	    out->InternalLink(username, page_get_title(page),
			      page_get_type(page)
			     );
    }
    else {
	/* link to not yet written WikiWord page */
	out->BrokenLink(username);
    }
}



static void
do_tarbackup ()
{
    char outstring[MAX_PATH];
    char date[MAX_DATELEN];

    get_shortdate(date);
    char* host = wiki_get_hostname();
    int port = wiki_get_port();

    sprintf(outstring, "http://%s:%i/Backup/pages-%s.tar", host, port, date);
    out->external_link(outstring, "Backup");
}



static void
do_date ()
{
    char buf[MAX_DATELEN];
    time_t acttime = time(NULL);

    strftime(buf, MAX_DATELEN-1, "%A, %d. %b. %Y", localtime(&acttime));
    buf[MAX_DATELEN-1] = '\0';
    out->Puts(buf);
}



static void
do_shortdate ()
{
    char buf[MAX_DATELEN];

    time_t acttime = time(NULL);
    strftime(buf, MAX_DATELEN-1, "%y%m%d", localtime(&acttime));
    buf[MAX_DATELEN-1] = '\0';
    out->Puts(buf);
}



static void
do_time ()
{
    char buf[MAX_TIMELEN];
    time_t acttime = time(NULL);

#ifdef	__OS2__
    strftime(buf, MAX_DATELEN-1, "%H:%M", localtime(&acttime));
#else
    strftime(buf, MAX_DATELEN-1, "%k:%M", localtime(&acttime));
#endif
    buf[MAX_DATELEN-1] = '\0';
    out->Puts(buf);
}



static void
do_wikistart ()
{
    char buf[MAX_TIMELEN];
    time_t starttime = wiki_get_starttime();

#ifdef	__OS2__
    strftime(buf, MAX_TIMELEN-1, "%A, %d. %b. %Y, %H:%M",
	     localtime(&starttime));
#else
    strftime(buf, MAX_TIMELEN-1, "%A, %d. %b. %Y, %k:%M",
	     localtime(&starttime));
#endif

    buf[MAX_DATELEN-1] = '\0';
    out->Puts(buf);
}



static void
do_wikiname ()
{
    out->Puts(wiki_get_wikiname());
}



static void
do_calls ()
{
    char buf [HTTP_MAX_LEN];

    sprintf(buf, "%i", wiki_get_calls() );
    out->Puts(buf);
}


/*
 * do_dailycalls - show the page calls per day
 */
static void
do_dailycalls ()
{
    char buf [HTTP_MAX_LEN];
    int days;

    /* never have zero days! */
    days = (time(NULL) - wiki_get_starttime()) / (60 * 60 * 24) + 1;
    sprintf(buf, "%i", wiki_get_calls() / days);
    out->Puts(buf);
}



static void
do_os ()
{
    char buf [HTTP_MAX_LEN];

    sprintf(buf, "%s %s", wiki_get_sysname(), wiki_get_release() );
    out->Puts(buf);
}



static void
do_machine ()
{
    out->Puts(wiki_get_machine());
}



static void
do_memusage ()
{
    char buf [HTTP_MAX_LEN];

    sprintf(buf, "%i Kb", pagelist_get_usedmemory() );
    out->Puts(buf);
}

static void
do_diskusage ()
{
    char buf [HTTP_MAX_LEN];

    sprintf(buf, "%i Kb", pagelist_get_useddisk() );
    out->Puts(buf);
}



static void
do_pagecount ()
{
    char buf [HTTP_MAX_LEN];

    sprintf(buf, "%i", pagelist_get_count() );
    out->Puts(buf);
}



static void
do_pagename ()
{
    char* name;

    name = var_get_val(server->variables, "page");
    if (name != NULL) {
	Page * page;

	page = pagelist_find_page(name);
	if (page) {
	    /* The page exists! */
	    if (page_is_seen(page))
		out->InternalLink(page_get_name(page),
				  page_get_title(page),
				  page_get_type(page)
				 );
	    else
		out->Puts(page_get_title(page));
	}
	else {
	    /* link to not yet written WikiWord page */
	    out->BrokenLink(name);
	}
    }
    else
	out->Puts("[PageName]");
}



static void
do_searchtext ()
{
    char* searchtext;

    searchtext = var_get_val(server->variables, "cutewiki-search");
    if (searchtext != NULL)
	out->Puts(searchtext);
    else
	out->Puts("[SearchText]");
}



static void
do_pwreset()
{
    if (user_is_admin()) {
	if (out == &htm) {
            /* just print something, if we are in HTML */
	    svr_printf(server, "<form method=\"post\" accept-charset=\"UTF-8\" action=\"/Password/Result\">\n");
	    svr_printf(server, "  <input type=\"text\" size=\"50\" name=\"pwreset\" value=\"\">\n");
	    svr_printf(server, "  <input class=\"content\" type=\"submit\" value=\" Password Reset \">\n");
	    svr_printf(server, "</form>\n");
	} else {
	    out->Puts("[ Password Reset Form ]");
	}
    }
    else {
	out->Puts("Just allowed for wiki admins!");
    }
}



static void
do_history ()
{
    char* pagename;

    pagename = var_get_val(server->variables, "page");
    if (pagename == NULL)
	out->Puts("[PageHistory]");
    else {
        /* revision, author, diff */
	out->TableBegin(3);
	out->TableHeadBegin();

#if GERMAN
	out->TableCellBegin();
	out->Puts("Von");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Nach");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Datum");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Geändert von");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Differenzen");
	out->TableCellEnd();
#else
	out->TableCellBegin();
	out->Puts("From");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("To");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Date");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Changed by");
	out->TableCellEnd();

	out->TableCellBegin();
	out->Puts("Diffs");
	out->TableCellEnd();
#endif
	out->TableHeadEnd();

	rcs_log(pagename);
        out->TableEnd();
    }
}



/*
 * do_history_line - print one line of a page's history
 *
 * This is called by rcs_log
 */
void
do_diff_boxhistory_line (char* rev1, char* rev2, char* changed)
{
}

static void
do_diffs ()
{
    char* pagename;
    char* rev1;
    char* rev2;

    pagename = var_get_val(server->variables, "page");
    rev1 = var_get_val(server->variables, "rev1");
    rev2 = var_get_val(server->variables, "rev2");
#if 0
    fprintf(stderr, "page: %s\n", pagename);
    fprintf(stderr, "rev1: %s\n", rev1);
    fprintf(stderr, "rev2: %s\n", rev2);
#endif
    if (pagename == NULL || rev1 == NULL || rev2 == NULL)
	out->Puts("[PageDiffs]");
    else
        rcs_diff(pagename, rev1, rev2);
}



#if 0
static void
do_revision ()
{
    char* revision;

    revision = var_get_val(server->variables, "revision");
    if (revision == NULL)
	out->Puts("[PageRevision]");
    else
	out->Puts(revision);
}
#endif


#if 0
static void
do_question()
{
    svr_printf(server, "<form method=\"post\" accept-charset=\"UTF-8\" action=\"/Support/Result\">\n");
    svr_printf(server, "  <input type=\"text\" size=\"50\" name=\"question\" value=\"\">\n");
    svr_printf(server, "  <input class=\"content\" type=\"submit\" value=\" Ask Now \">\n");
    svr_printf(server, "</form>\n");
}



static void
do_answer(ParseState* fmt)
{
    char question[1024];

    question = var_get_val(server->variables, "question");
    if (question != NULL) {
        do_string(robo_ask("Martin", question), fmt);
    }
    else
        do_string("Please ask your question. [question] ", fmt);
}
#endif


/*
 * do_quote - accept part of text beginning '
 *
 * returns the length, which is the count of quotes.
 */
static void
do_quote(char** line, LineFmt* fmt)
{
    char* lp;
    int	quotes;

    lp = *line;
    quotes = 0;
    while (*lp == '\'') {
        quotes++;
        lp++;
    }

    if (quotes == 1) {
        out->Putc('\'');
    }
    else {
        if (quotes != 3) {	// 2 or bigger switches italic
            if (!fmt->italic)
                out->ItalicBegin();
            else
                out->ItalicEnd();
            fmt->italic = !fmt->italic;
        }
        if (quotes != 2) {	// 3 or bigger switches bold
            if (!fmt->bold)
                out->BoldBegin();
            else
                out->BoldEnd();
            fmt->bold = !fmt->bold;
        }
    }
    *line = lp;
}



/*
 * do_alpha - accept part of text beginning with a letter
 */
static void
do_alpha(char** line, LineFmt* fmt)
{
    char* lp;
    char* word;

    lp = *line;
    word = get_alnum(&lp);
    if (is_wikiword(word)) {
        Page*	page;

        page = pagelist_find_page(word);
        if (page != NULL) {
            if (page_is_seen(page))
		out->InternalLink(page_get_name(page),
				  page_get_title(page),
				  page_get_type(page)
				 );
            else
                out->Puts(page_get_title(page));
        }
        else {
            /* link to not yet written WikiWord page */
            out->BrokenLink(word);
        }
    }
    else if ((*lp == ':') && is_url(word)) {
        /* it is an URL */
        lp = *line;             /* back to start of URL */
        free(word);
        word = get_url(&lp);
        out->url(word);     /* is a URL - check protocol */
    }
    else {
        out->Puts(word);
    }

    free(word);
    *line = lp;
}



/*
 * do_square - accept part of text beginning with [
 */
static void
do_square(char** line, LineFmt* fmt, ParseState * pfmt)
{
    char* lp;
    bool done;

    done = true;
    lp = *line;
    lp++;                     /* jump over [ */

    if (isdigit((unsigned char)*lp)) {
        /* for shure, this is a footnote */
        char*	footnote;

	while (isdigit((unsigned char)*lp))
	    lp++;
        get_space(&lp);
        footnote = get_square(&lp);
        out->Footnote(footnote);
	free(footnote);
    }
    else if (islower((unsigned char)*lp)) {
	char*	word;

        word = get_alnum(&lp);
        switch (*lp) {
        case ':':
            /* check, if it was an URL kind of string */
            if (is_url(word)) {
                lp -= strlen(word);		// back up to start of URL
                free(word);
		word = get_url(&lp);

                get_space(&lp);
                if (*lp == ']') {
                    /* is a URL - check protocol */
                    out->image_url(word);
                }
                else {
                    char * text;

                    text = get_square(&lp);	/* jumps over ] */
                    out->external_link(word, text);
                    free(text);
                }
	    }
	    else
		done = false;

            break;

        case '=':
	    /* may be, it's a dynamic list */
            if (!strcmp(word, "pages")) {
                lp++;
                free(word);
                word = get_square(&lp);
		do_list(pagelist_search_title(word, NULL), SHOW_DATE|SHOW_OWNER);
            }
            else if (!strcmp(word, "topic")) {
                lp++;
		free(word);
                word = get_square(&lp);
                do_list(pagelist_search_topic(word), SHOW_DATE|SHOW_OWNER);
            }
	    else if (!strcmp(word, "category")) {
                lp++;
		free(word);
                word = get_square(&lp);
                do_list(pagelist_in_category(word), SHOW_DATE|SHOW_OWNER);
            }
	    else
		done = false;
	    break;

        default:
            /* see, if the word is an image */
	    if (is_image(word))
		out->image(word);
	    else
		done = false;
	}
	free(word);
    }
    else if (isupper((unsigned char)*lp)) {
	char*	word;

	/* see, if the word is one of the special commands */
	word = get_square(&lp);
	if (strcmp(word, "RecentChanges") == 0)
	    do_changes();
	else if (strcmp(word, "EditForm") == 0)
	    do_editform();
	else if (strcmp(word, "PageIndex") == 0)
	    do_index();
	else if (strcmp(word, "PageList") == 0)
	    do_pagelist();
	else if (strcmp(word, "UserList") == 0)
	    do_userlist();
	else if (strcmp(word, "GroupList") == 0)
	    do_grouplist();
	else if (strcmp(word, "PageHistory") == 0)
	    do_history();
	else if (strcmp(word, "PageDiffs") == 0)
	    do_diffs();
	else if (strcmp(word, "ReverseList") == 0)
	    do_reverselist();
	else if (strcmp(word, "SearchList") == 0)
	    do_searchlist();
	else if (strcmp(word, "CategoryList") == 0)
	    do_categorylist();
	else if (strcmp(word, "SearchText") == 0)
	    do_searchtext();
#if 0
	else if (strcmp(word, "SupportAnswer") == 0)
	    do_answer(pfmt);
	else if (strcmp(word, "SupportQuestion") == 0)
	    do_question();
#endif
	else if (strcmp(word, "PageSource") == 0)
	    do_sourceform();
	else if (strcmp(word, "PasswordReset") == 0)
	    do_pwreset();
	else if (strcmp(word, "MainMemory") == 0)
	    do_memusage();
	else if (strcmp(word, "DiskUsage") == 0)
	    do_diskusage();
	else if (strcmp(word, "PageCount") == 0)
	    do_pagecount();
	else if (strcmp(word, "PageName") == 0)
	    do_pagename();
	else if (strcmp(word, "WikiName") == 0)
	    do_wikiname();
	else if (strcmp(word, "OperatingSystem") == 0)
	    do_os();
	else if (strcmp(word, "MachineName") == 0)
	    do_machine();
	else if (strcmp(word, "UserName") == 0)
	    do_user();
	else if (strcmp(word, "ActualDate") == 0)
	    do_date();
	else if (strcmp(word, "ActualTime") == 0)
	    do_time();
	else if (strcmp(word, "ShortDate") == 0)
	    do_shortdate();
	else if (strcmp(word, "TarBackup") == 0)
	    do_tarbackup();
	else if (strcmp(word, "ErrorMessage") == 0)
	    do_errormsg();
	else if (strcmp(word, "ErrorDescription") == 0)
	    do_errordsc();
	else if (strcmp(word, "PageCalls") == 0)
	    do_calls();
	else if (strcmp(word, "DailyCalls") == 0)
	    do_dailycalls();
	else if (strcmp(word, "WikiStart") == 0)
	    do_wikistart();
	else
	    done = false;

	free(word);
    }
    else
        done = false;

    if (done == false) {
	lp = *line;             /* forget it all */
	lp++;                   /* jump over [ */
        out->Putc('[');
    }
    else {
	if (*lp)
	    lp++;       /* skip the closing ']', if there */
    }

    *line = lp;
}


/*
 * do_pipe - accept part of text beginning |
 */
static void
do_pipe(char** line, ParseState * pfmt)
{
    char* lp;

    lp = *line;
    lp++;       /* jump over | */

    if (pfmt->table ) {
        get_space(&lp);        /* strip leading spaces */
        if (is_cell(lp))
            do_cell(&lp, pfmt);
        get_space(&lp);
    }
    else {
        /* we are not in a Table */
        out->Putc('|');
    }
    *line = lp;
}



/*
 * do_stringstart - write
 */
static void
do_linestart(ParseState * state)
{
    /* write needed formatting for the line output */
    if (state->indent)
        out->ListItemBegin();

    else if (state->table) {     /* table row */
        if (state->table == 2)      /* table header */
            out->TableHeadBegin();
        else
            out->TableRowBegin();
    }
    else
	out->LineBegin();
}

/*
 * do_lineend - write
 */
static void
do_lineend(ParseState * state)
{
    if (!state->table && !state->indent)
	out->LineEnd();

    /* write line ending formats */
    if (state->table) {     		/* table row */
        if (state->table == 2) {		/* table header */
            out->TableHeadEnd();
            state->table = 1;
        }
        else
            out->TableRowEnd();
    }
    else if (state->indent)
        out->ListItemEnd();
}



/*
 * do_string - write a string of text
 *
 * For different paragraph specific formattings, like table, preformatted
 * text and headings, the actual paragraph state needs to be passed.
 */
static void
do_string(char* lp, ParseState * state)
{
    LineFmt	fmt;
    char ch;

    /* Now write the real characters of the line */
    fmt.italic = (state->quote & 1) ? true : false;
    fmt.bold = false;
    fmt.number = false;

    if (fmt.italic)
        out->ItalicBegin();

    /* No formatting, if preformatted text or heading */
    if ( state->head || state->pre) {
        out->Puts(lp);
    }
    else {
        while ((ch = *lp)) {
            switch (ch) {
            case '\'':
                do_quote(&lp, &fmt);
                break;

            case '[':
                do_square(&lp, &fmt, state);
                break;

            case '|':
                do_pipe(&lp, state);
                break;

            default:
                if (isalpha((unsigned char)ch)) {
                    do_alpha(&lp, &fmt);
                }
                else {
                    out->Putc(ch);
                    lp++;
                }
            }
        }
    }

    /* these formats are just valid for ONE line, so end them  */
    if (fmt.bold)
        out->BoldEnd();
    if (fmt.italic)
        out->ItalicEnd();
}



/*
 * set_state - set start and end for different paragraph types
 *
 * Types can be normal, lists, horizontal rulers, footnoteblocks,...
 *
 * beg_state - actual settings for starting the paragraph
 * end_state - actual settings for ending the paragraph
 */
static void
set_state(ParseState* beg_state, ParseState* end_state)
{
    // turn ruler on
    if (beg_state->ruler && !end_state->ruler) {
        out->RulerEnd();
        beg_state->ruler = false;
    }

    // turn table formatting off
    if (beg_state->table && !end_state->table) {
        out->TableEnd();
        beg_state->table = 0;
    }

    // turn heading off
    if (beg_state->head && !end_state->head) {
        out->HeadingEnd(beg_state->head);
        beg_state->head = 0;
    }

    // turn block-quotes off
    while (end_state->quote < beg_state->quote) {
        out->BlockquoteEnd();
        beg_state->quote--;
    }
    // turn preformatting off
    if (beg_state->pre && !end_state->pre) {
        out->PreEnd();
        beg_state->pre = false;
    }

    // end paragraph
    if (beg_state->para && !end_state->para) {
        out->ParaEnd();
        beg_state->para = false;
    }

    // reduce indent, if we had not been in preformatted before
    while (end_state->indent < beg_state->indent) {
        beg_state->indent--;
        if (beg_state->indents[beg_state->indent] == 'o')
            out->NumListEnd();
        else
            out->ListEnd();
    }

    // increase indent, if we had not been in preformatted before
    while (end_state->indent > beg_state->indent) {
        beg_state->indents[beg_state->indent] = end_state->indents[beg_state->indent];
        if (beg_state->indents[beg_state->indent] == 'o')
            out->NumListBegin();
        else
            out->ListBegin();
        beg_state->indent++;
    }

    // begin paragraph
    if (end_state->para && !beg_state->para) {
        out->ParaBegin();
        beg_state->para = true;
    }

    // turn preformatting on, if we had not been in a list before
    if (end_state->pre && !beg_state->pre) {
        out->PreBegin();
        beg_state->pre = true;
    }
    // turn block-quotes on
    while (end_state->quote > beg_state->quote)
    {
        out->BlockquoteBegin();
        beg_state->quote++;
    }

    // turn heading on
    if (end_state->head && !beg_state->head)
    {
        out->HeadingBegin(end_state->head);
        beg_state->head = end_state->head;
    }

    // start table formatting
    if (end_state->table && !beg_state->table) {
        out->TableBegin(end_state->cells);
        beg_state->table = end_state->table;
    }

    // start ruler
    if (end_state->ruler && !beg_state->ruler) {
        out->RulerBegin();
        beg_state->ruler = end_state->ruler;
    }
}

static void
reset_state(ParseState * state)
{
    state->indent = 0;
    state->quote = 0;
    state->head = 0;
    state->table = 0;
    state->cells = 0;
    state->pre = false;
    state->para = false;
    state->ruler = false;
}



static char*
change_state(char* line, ParseState * state)
{
    ParseState	newstate;
    int		spaces;
    char* lp = line;

    /* if it's a comment, just skip the line and don't change the state */
    state->comment = false;
    if (*lp ==  '#') {
        state->comment = true;
        return lp;
    }

    reset_state(&newstate);

    switch (*lp) {
    case '\0':
        /* empty line - change nothing */
        break;

    case ' ':
        /* count leading spaces */
        spaces = 0;
        while (*lp == ' ') {
            lp++;
            spaces++;
        }

        if (spaces) {
            /* at first assume prefromatted */
            newstate.pre = true;

            if (spaces < 4) {
                /* do not do a list, if we had been in preformatted mode */
                if (!state->pre) {
                    if ((lp[0] == '*') && (lp[1] == ' ')) {
                        /* do bulleted list*/
                        lp += 2;
                        newstate.indent = spaces;
                        memset(newstate.indents, 'u', spaces);
                        newstate.pre = false;
                    }
                    else if (isdigit((unsigned char)lp[0])) {
                        /* do numbered list*/
                        char*	ep = lp;

                        while (isdigit((unsigned char)*ep))
                            ep++;
                        if ((ep[0] == '.') && (ep[1] == ' ')) {
                            lp = ep + 2;
                            newstate.indent = spaces;
                            memset(newstate.indents, 'o', spaces);
                            newstate.pre = false;
                        }
                    }
                }
            }
            // preserve indentation
            if (newstate.pre) {
                while (--spaces) {
                    --lp;
                }
            }
        }
        break;

    case '=':
        /* count characters for heading level */
        do {
            lp++;
            newstate.head++;
        } while (*lp == '=');

        get_space(&lp);

        switch (newstate.head) {
        case 1:
            newstate.head = 3;
            break;

        case 2:
            newstate.head = 2;
            break;

        default:
            /* maximum heading level is 2 */
            newstate.head = 1;
            break;
        }
        break;

    case '|':
        /* we have a table */
        newstate.table++;
        if (lp[1] == '|') {
            /* this is a table heading */
            newstate.table++;
            lp++;
        }
        count_cells(lp, &newstate);
        break;

    case '>':
        do {
            lp++;
            newstate.quote++;
            get_space(&lp);
        } while (*lp == '>');
        break;

    case '-':
        {
            char*	lp2;

            lp2 = lp;
            while (*lp2 == '-')
                lp2++;
            if (!*lp2 && (lp2 - lp >= 4)) {
                newstate.ruler = true;
            }
        }
        break;

    default:
        /* if indent or normal text, then begin paragraph */
        newstate.para = true;
    }
    set_state(state, &newstate);

    return lp;
}



/*
 * write out a line
 */
static void
do_line(char ** text, ParseState * state)
{
    char* line;
    char* string;

    /* get next line */
    line = get_line(text);

    /* set attributes for line and return where we are */
    string = change_state(line, state);

    /* write line, if it is no ruler */
    if (!state->ruler && !state->comment) {
        do_linestart(state);
        do_string(string, state);
        do_lineend(state);
    }
    free(line);
}



/*
 * OutputPage -  write out a page with a choosen output option
 *
 * load the page text and print it line by line
*/
static void
do_page(Page * page, int mode)
{
    char*	text;
    bool 	loaded;
    ParseState	state;
    ParseState	newstate;

    page_load_text(page, &loaded);
    out->page_header(page, mode);

    reset_state(&state);
    text = page_get_text(page);
    if (text) {
        while (*text)
            do_line(&text, &state);
    }

    /* end the last paragraph, list or table */
    reset_state(&newstate);
    set_state(&state, &newstate);

    out->page_footer(page, mode);
    page_unload_text(page, loaded);
}



/*
 * OutputPage -  write out a page with a choosen output option
 *
 * The output option is some kind of pluggable driver for HTML or RTF.
 * The page itself must exist!
 */
void
out_print_page(Page * page, int mode)
{

    switch(mode) {
#if 0
    case MODE_RSS:
	out = &rss;
	break;
#endif
    case MODE_RTF:
	out = &rtf;
	break;
    case MODE_PRINT:
	out = &prt;
	break;
    default:
	out = &htm;     /* also for edit mode */
    }
    do_page(page, mode);
    out = &htm;        /* Reset to normal HTML operation */
}



static void
set_category(char * addition)
{
    char* category;

    /* Handle the hidden category setting */
    category = var_get_val(server->variables, "cutewiki-category");

#if 0
    printf("============ set_category!\n");
    printf("category = %s\n", category);
    printf("addition = %s\n", addition);
#endif

    if (category != NULL) {
	if (addition != NULL) {
	    char result[MAX_WIKINAME*2];

	    /* add addition to existent category in cookie and vars */
	    snprintf(result, MAX_WIKINAME*2, "%s+%s", category, addition );
	    svr_set_cookie(server, "cutewiki-category", result);
	    var_del(&server->variables, "cutewiki-category");
	    var_set(&server->variables, "cutewiki-category", result);
	}
    }
    else {
	if (addition != NULL) {
	    /* set just cookie and vars for new category */
	    svr_set_cookie(server, "cutewiki-category", addition);
	    var_set(&server->variables, "cutewiki-category", addition);
	}
    }
}


/*
 * OutputPage -  write out a page with a choosen output option
 *
 * The output option is some kind of pluggable driver for HTML or RTF
 */
void
out_write_page(char * name, int mode)
{
    Page * 	page;

    page = pagelist_find_page(name);    // see, if we can find page
    if (page) {
        /* if page is not allowed to see, take StartPage */
	if (!page_is_seen(page)) {
            svr_set_response(server, "403");    /* forbidden */
	    out_write_error("403",
			    "You are not allowed to view this page!",
			    "It may be the case, that somebody did "
			    "change the permissions, and you just tried "
			    "to access the page afterwards.\n\n"

			    "Also it may be, that you are one of these "
			    "bad guys that want to see things that you "
			    "better should not see. If you just would "
			    "have heard on your mom...");
        }

	if (page_is_category(page))
	    set_category(name);

	out_print_page(page, mode);
    }
    else {
        /* if it is an elemental page, create it automatically */
	if (create_is_special(name)) {
	    if (create_special_page(name)) {
		out_write_page(name, MODE_NORMAL);
	    }
	    else {
                fprintf(stderr, "Page = %s\n", name);
		svr_set_response(server, "204");        /* no content */
	    }
	}
	else {
	    /* If page did not yet exist make one */
	    var_set(&server->variables, "page", name);
	    out_write_page("EditPage", MODE_EDIT);
	}
    }
}



/*
 * out_write_error -  write an error message 
 *
 * This displays a short error message and a description
 */
void
out_write_error(char * num, char * msg, char * dsc)
{
    svr_set_response(server, num);
    var_set(&server->variables, "errormsg", msg);
    var_set(&server->variables, "errordsc", dsc);
    out_write_page("ErrorPage", MODE_NORMAL);
}

