/*
 * rcs.c - rcs integration for CuteWiki
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>

#include "types.h"
#include "config.h"
#include "cutewiki.h"
#include "page_list.h"
#include "parser.h"



static bool available;



/*
 * rcs_is_available - check for availability of RCS
 *
 * Check, if ci, rlog and rcsdiff is in PATH and executable.
 */
static bool
rcs_is_available()
{
    char command[MAX_PATH];

    /* check for ci */
    snprintf(command, MAX_PATH, "ci -V >/dev/null");
    if (system(command) == -1)
	return false;

    /* check for rlog */
    snprintf(command, MAX_PATH, "rlog -V >/dev/null");
    if (system(command) == -1)
	return false;

    /* check for rcsdiff */
    snprintf(command, MAX_PATH, "rcsdiff -V >/dev/null");
    if (system(command) == -1)
	return false;

    return true;
}



/*
 * rcs_checkin - check in a page to RCS
 *
 * return true on success, false otherwise
 */
bool
rcs_checkin (char* pagename, char* username)
{
    char command[MAX_PATH];

    snprintf(command, MAX_PATH, "ci -q -l "
	     "-t-'%s' -m'user: %s' %s/%s.wik >/dev/null 2>&1",
	     pagename, username, pagepath, pagename);
    system(command);

    return true;
}



static void
rcs_print_user(char* username) {
    Page * page;

    page = pagelist_find_page(username);
    if (page) {
	/* The page exists! */
	if (!page_is_seen(page))
	    out->Puts(page_get_title(page));
	else
	    out->InternalLink(username, page_get_title(page),
			      page_get_type(page));
    }
    else {
	/* link to not yet written WikiWord page */
	out->BrokenLink(username);
    }
}



/*
 * rcs_print_diff - print out the "diff"-link on the page
 */
static void
rcs_print_diff(char* pagename, char* rev1, char* rev2) {
    if (pagename == NULL || rev1 == NULL || rev2 == NULL)
	svr_puts(server, "diff\n");
    else
	svr_printf(server, "<a class=\"gotopage\" href=\"/Diff/%s?rev1=%s&rev2=%s\" "
		   "title=\"%s - %s\">diff</a>\n",
		   pagename, rev1, rev2, rev1, rev2);
}



/*
 * rcs_log - call rcslog and process it's output
 *
 * Draws a table's lines, one by one.
 */
bool
rcs_log (char* pagename)
{
    FILE *pipe;
    char command[MAX_PATH];
    char line[1024];
    char* token;
    char rev_from[64];    	/* store the original revision */
    char rev_to[64];		/* store the changed revision */
    char date   [MAX_WIKINAME];	/* store the date */
    char user[MAX_WIKINAME];	/* store the one who did change */

    snprintf(command, MAX_PATH, "rlog -zLT %s/%s.wik", pagepath, pagename);
    pipe = popen(command, "r");
    if (pipe == NULL)
	return false;

    rev_from[0] = '\0';
    rev_to[0] = '\0';
    date[0] = '\0';
    user[0] = '\0';

    while (fgets(line, sizeof(line), pipe) != NULL) {
	token = strtok(line, " ");
	if (token != NULL) {
	    if (strcmp("revision", token) == 0) {
		strcpy(rev_from, strtok(NULL, " \t\n"));

                /* if not last version, print the line */
		if (strlen(rev_to) > 0) {
		    out->TableRowBegin();

		    out->TableNumberBegin();
		    out->Puts(rev_from);
		    out->TableNumberEnd();

		    out->TableNumberBegin();
		    out->Puts(rev_to);
		    out->TableNumberEnd();

		    out->TableCellBegin();
		    out->Puts(date);
		    out->TableCellEnd();

		    out->TableCellBegin();
                    rcs_print_user(user);
		    out->TableCellEnd();

		    out->TableCellBegin();
                    rcs_print_diff(pagename, rev_from, rev_to);
		    out->TableCellEnd();

		    out->TableRowEnd();
		}
		strcpy(rev_to, rev_from); /* save older revision */
	    }
	    else if (strcmp("date:", token) == 0) {
		strcpy(date, strtok(NULL, ";+"));
	    }
	    else if (strcmp("user:", token) == 0) {
		strcpy(user, strtok(NULL, ";+\n"));
	    }
	}
    }
    fflush(pipe);
    pclose(pipe);

    return false;
}


/*
 * rcs_diff - get the diffs for a specific page revision
 *
 * return true on success, false otherwise
 */
bool
rcs_diff (char* pagename, char* revision1, char* revision2)
{
    FILE *pipe;
    char line[MAX_TEXTLEN];
    //char *line;
    char command[MAX_PATH];
    enum ecolor {COL_RED, COL_BLUE, COL_GREEN};
    enum {header, begin, before, after};
    enum {normal, added, deleted, changed};
    int block_type = header;
    int block_old = header;
    int line_type = normal;
    int line_old = normal;

    snprintf(command, MAX_PATH, "rcsdiff -q -c -zLT "
	     "-r%s -r%s %s/%s.wik",
	     revision1, revision2, pagepath, pagename);

    pipe = popen(command, "r");
    if (pipe == NULL)
        return false;

    svr_puts(server, "<div class=\"diff\">\n");
    svr_puts(server, "<div class=\"head\">\n");
    while (fgets(line, sizeof(line), pipe) != NULL) {

        /* did our block_type change? */
	if (strncmp(line, "****", 4) == 0) {
	    block_type = begin;
	}
	else if (block_type != header && strncmp(line, "*** ", 4) == 0) {
	    block_type = before;
            continue;
	}
	else if (block_type != header && strncmp(line, "--- ", 4) == 0) {
	    block_type = after;
	    continue;
	}

        /* did our line_type change? */
	if (strncmp(line, "+ ", 2) == 0) {
	    line_type = added;
	}
	else if (strncmp(line, "- ", 2) == 0) {
	    line_type = deleted;
	}
	else if (strncmp(line, "! ", 2) == 0) {
	    line_type = changed;
	}
	else {
            line_type = normal;
	}

        /* react on new block_type, if changed */
	if (block_type != block_old) {
	    svr_puts(server, "</div>\n");

	    if (block_type != begin) {
		out->RulerBegin();
		out->RulerEnd();
	    }

	    /* print it */
	    switch (block_type) {
	    case before:
		svr_puts(server, "<div class=\"old\">\n");
		break;
	    case after:
		svr_puts(server, "<div class=\"new\">\n");
		break;
	    default:
		svr_puts(server, "<div class=\"head\">\n");
	    }
	}

        /* react on new line_type, if changed */
	if (line_type != line_old) {
	    /* print it */
	    switch (line_type) {
	    case added:
		svr_puts(server, "<div class=\"added\">\n");
		break;
	    case deleted:
		svr_puts(server, "<div class=\"deleted\">\n");
		break;
	    case changed:
		svr_puts(server, "<div class=\"changed\">\n");
		break;
	    default:
		svr_puts(server, "</div>\n");
	    }
	}

        /* print the actual line of RCS output */
	switch (block_type) {
	case header:
	    out->Puts(line+4);
	    svr_puts(server, "<br>\n");
            break;
	case begin:
	    break;
	case before:
	    if (block_type == block_old) {
		out->Puts(line+2);
		svr_puts(server, "<br>\n");
	    }
	    break;
	case after:
	    if (block_type == block_old) {
		out->Puts(line+2);
		svr_puts(server, "<br>\n");
	    }
            break;
	default:
	    out->Puts(line+2);
	    svr_puts(server, "<br>\n");
	}

        /* save old state for next turn */
        block_old = block_type;
        line_old = line_type;
    }
    svr_puts(server, "</div>\n");
    svr_puts(server, "</div>");

    /* Executing a pclose() on an un-flushed popen() read stream can cause
     * the shell(1) started by the popen() to issue a 'Broken Pipe' error
     * if the actual command has finished, but the parent shell has not yet
     * written the child's output back on the pipe (system buffers).
     */
    fflush(pipe);
    pclose(pipe);

    return true;
}



bool
rcs_available ()
{
    return available;
}



void
rcs_init ()
{
    if (rcs_is_available()) {
	/* we did find all needed RCS commands */
	available = true;
	fprintf(stderr, "Info:  RCS is available and will be used.\n");
    }
    else {
	/* we did not find all needed RCS commands */
	available = false;
	fprintf(stderr, "Info:  RCS not found, no version management.\n");
    }
;

}
