/*
 * out-rss.c - output functions for use in rss-2.0 description tag
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "config.h"
#include "svr.h"
#include "parser.h"
#include "misc.h"


/*
 * With this flag we try to output just the first paragraph,
 * so that a reader of an RSS feed does not get the whole page
 * to read - therefore he get's a link.  :-)
 */
bool ready;		/* true, when first paragraph printed */
bool first_heading;      /* print just the first heading */


/*
 * rss_putc - outputs a character to the server
 *
 * in RSS we have XML, so we need to encode here...
 */
void
rss_putc(char ch)
{
    if (!ready) {
	switch (ch) {
	case '&':
	    svr_puts(server, "&amp;");
	    break;
        case '\'':
	    svr_puts(server, "&apos;");
	    break;
        case '\"':
	    svr_puts(server, "&quot;");
	    break;
	case '<':
	    svr_puts(server, "&lt;");
	    break;
	case '>':
	    svr_puts(server, "&gt;");
	    break;
	case '%':
	    svr_puts(server, "&#x25;");
	    break;
	case 0x0d:
	    // skip dos like line ending
	    break;
	default:
	    svr_putc(server, ch);
	}
    }
}



/*
 * rss_Puts - outputs a string to the server
 */
void
rss_puts(char * str)
{
    if (!ready) {
	if (str) {
	    char ch;

	    while ((ch = *str++)) {
		rss_putc(ch);
	    }
	}
    }
}



/*
 * rss_PageHeader
 *
 * No real title is printed, because it is already in the xml-file
 */
void
rss_page_header(Page * page, int mode)
{
    ready = false;              /* initially we want to see something */
    first_heading = true;
}



void
rss_page_footer(Page * page, int mode)
{
    ready = true;               /* stop printing */
}



void
rss_ParaBegin()
{
}

void
rss_ParaEnd()
{
    ready = true;               /* stop printing */
}

void
rss_PreBegin()
{
}

void
rss_PreEnd()
{
    ready = true;               /* stop printing */
}

void
rss_BlockquoteBegin()
{
}

void
rss_BlockquoteEnd()
{
}

void
rss_RulerBegin()
{
    ready = true;               /* stop printing, no lists */
}

void
rss_RulerEnd()
{
    ready = true;               /* stop printing */
}

void
rss_ListBegin()
{
}

void
rss_ListEnd()
{
    ready = true;               /* stop printing */
}

void
rss_NumListBegin()
{
    ready = true;               /* stop printing, no lists */
}

void
rss_NumListEnd()
{
}

void
rss_ListItemBegin()
{
}

void
rss_ListItemEnd()
{
}

void
rss_LineBegin()
{
}

void
rss_LineEnd()
{
    svr_printf(server, "\n");
}

void
rss_HeadingBegin(int level)
{
    if (!first_heading)         /* end, if second heading */
        ready = true;
}

void
rss_HeadingEnd(int level)
{
    first_heading = false;
    if (!first_heading)         /* end, if second heading */
	ready = true;
}

void
rss_footnote(char * note)
{
}

void
rss_BoldBegin()
{
}

void
rss_BoldEnd()
{
}

void
rss_ItalicBegin()
{
}

void
rss_ItalicEnd()
{
}

void
rss_InternalLink(char * name, char * title, Pagetype type)
{
    svr_printf(server, "%s", title);
}

void
rss_BrokenLink(char * name)
{
    svr_printf(server, "%s", name);
}

void
rss_TableBegin(int cells)
{
    ready = true;               /* stop at table start */
}

void
rss_TableEnd()
{
}

void
rss_TableHeadBegin()
{
}

void
rss_TableHeadEnd()
{
}

void
rss_TableRowBegin()
{
}

void
rss_TableRowEnd()
{
}

void
rss_TableCellBegin()
{
}

void
rss_TableCellEnd()
{
}

void
rss_TableNumberBegin()
{
}

void
rss_TableNumberEnd()
{
}

void
rss_url(char * url)
{
    svr_printf(server, "%s", url);
}

void
rss_external_link(char * url, char * text)
{
    rss_puts(text);
}



void
rss_image_url(char * url)
{
}

void
rss_image(char * image)
{
}



/* pluggable output module for RSS-Mode */
Output rss = {
    rss_putc,
    rss_puts,
    rss_page_header,
    rss_page_footer,
    rss_ParaBegin,
    rss_ParaEnd,
    rss_PreBegin,
    rss_PreEnd,
    rss_BlockquoteBegin,
    rss_BlockquoteEnd,
    rss_RulerBegin,
    rss_RulerEnd,
    rss_ListBegin,
    rss_ListEnd,
    rss_NumListBegin,
    rss_NumListEnd,
    rss_ListItemBegin,
    rss_ListItemEnd,
    rss_LineBegin,
    rss_LineEnd,
    rss_HeadingBegin,
    rss_HeadingEnd,
    rss_footnote,
    rss_BoldBegin,
    rss_BoldEnd,
    rss_ItalicBegin,
    rss_ItalicEnd,
    rss_InternalLink,
    rss_BrokenLink,
    rss_TableBegin,
    rss_TableEnd,
    rss_TableHeadBegin,
    rss_TableHeadEnd,
    rss_TableRowBegin,
    rss_TableRowEnd,
    rss_TableCellBegin,
    rss_TableCellEnd,
    rss_TableNumberBegin,
    rss_TableNumberEnd,
    rss_image,
    rss_url,
    rss_image_url,
    rss_external_link
};
