/*
 * out-html.c - output functions for HTML
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "cutewiki.h"
#include "svr.h"
#include "request.h"
#include "parser.h"
#include "menu.h"
#include "misc.h"


/* Avoid passing it all the time */

static int numfoot;                 	/* counted number of footnotes */
static char * footnote [100];		/* pointer to footnotes */
static int tableheader = 0;		/* header or normal table row */
static int indentlevel = 0;

/* internal prototypes */
void	html_ruler_begin();



/*
 * xml_putc - outputs a character to the server
 */
void
html_putc(char ch)
{
    xml_putc(ch);
}



/*
 * html_Puts - outputs a string to the server
 */
void
html_puts(char * str)
{
    xml_puts(str);
}


static void
html_footnotes()
{
    int i;

    html_ruler_begin();

    svr_puts(server, "<div class=\"footnotes\">\n");
    for (i=1; i<=numfoot; i++) {
        svr_printf(server, "<a name=\"%d\">[%d]</a> ", i, i);
        html_puts(footnote[i]);
        free(footnote[i]);
        svr_puts(server, "<br>\n");
    }
    svr_puts(server, "</div>\n");
}


static void
html_InfoLine(char * string)
{
    svr_printf(server, "<script type=\"text/javascript\">\n"
                "<!--\n"
                "window.status = \"%s\"\n"
                "//-->\n"
                "</script>\n", string);
}


void
html_page_header(Page * page, int mode)
{
    numfoot = 0;        /* reset footnote counter */
    char * topic = page_get_topic(page);
    char * topictitle = page_find_title(topic);
    char * name = page_get_name(page);
    char * title = page_get_title(page);

    svr_use_utf8(true);
    svr_puts(server, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    
    svr_puts(server, "<html>\n\n<head>\n");
    svr_puts(server, "  <link rel=\"stylesheet\" type=\"text/css\" href=\"/Files/cwhtml.css\">\n");

    /* special section for going around bugs of IE */
    svr_puts(server, "  <!--[if gte IE 5]>\n");
    svr_puts(server, "  <link rel=\"stylesheet\" type=\"text/css\" href=\"/Files/cwhtml_ie.css\">\n");
    svr_puts(server, "  <![endif]-->\n");

    svr_puts(server, "  <link rel=\"icon\" href=\"/Files/cutewiki.ico\" type=\"image/ico\">\n");
    svr_puts(server, "  <link rel=\"shortcut icon\" href=\"/Files/cutewiki.ico\">\n");

    svr_puts(server, "  <meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\">\n");
    svr_puts(server, "  <meta http-equiv=\"cache-control\" content=\"no-store\" >\n");
    svr_puts(server, "  <meta http-equiv=\"pragma\" content=\"no-cache\" >\n");
    svr_puts(server, "  <meta http-equiv=\"expires\" content=\"0\" >\n");
#if GERMAN
    svr_puts(server, "  <meta http-equiv=\"content-language\" content=\"de\">\n");
#else
    svr_puts(server, "  <meta http-equiv=\"content-language\" content=\"en\">\n");
#endif

    svr_puts(server, "  <meta name=\"robots\" content=\"noindex\">\n");
    svr_puts(server, "  <meta name=\"generator\" content=\"CuteWiki\">\n");

    svr_printf(server, "  <title>%s</title>\n", title);
    svr_puts(server, "</head>\n\n\n");

    if (mode != MODE_EDIT) {
	svr_printf(server, "<body ondblclick=\"document.location.href='/Edit/%s'; \">\n\n", name);
	if (mode == MODE_ASCII)
	    svr_puts(server, "<body style=\"background-color:#eeeeee;\">\n\n");
    }
    else
	svr_puts(server, "<body style=\"background-color:#eeeeee;\">\n\n");

    /* Print the toolbar and also get view mode */
    menu_write_bar(name, mode);

    svr_puts(server, "<div class=\"text\">\n");
    svr_puts(server, "<div class=\"header\">\n");

    html_InfoLine(title);

    switch(page_get_type(page)) {
    case PT_USER:
	svr_printf(server, "<img title=\"Homepage\" alt=\"Homepage\" "
		   "src=\"/Images/person.png\">\n");
        break;
    case PT_GROUP:
	svr_printf(server, "<img title=\"Grouppage\" alt=\"Grouppage\" "
		   "src=\"/Images/people.png\">\n");
        break;
    case PT_CATEGORY:
	svr_printf(server, "<img title=\"Category\" alt=\"Category\" "
		   "src=\"/Images/category.png\">\n");
	break;
    case PT_NORMAL:
	break;
    }
#if GERMAN
    svr_printf(server, "<a href=\"/Reverse/%s\" title=\"Zeige Verweise auf \'%s\'\">%s</a>\n",
	       name, title, title);
#else
    svr_printf(server, "<a href=\"/Reverse/%s\" title=\"Reverse Lookup of %s\">%s</a>\n",
	       name, title, title);
#endif

    if (topic && (strlen(topictitle) > 0)) {
        svr_puts(server, "<span class=\"topic\">");
#if GERMAN
        svr_printf(server, "  /  <a href=\"/Wiki/%s\" title=\"Thema\">%s</a>", topic, topictitle);
#else
        svr_printf(server, "  /  <a href=\"/Wiki/%s\" title=\"Topic\">%s</a>", topic, topictitle);
#endif
        svr_puts(server, "</span>");
    }

    svr_puts(server, "</div>\n\n<div class=\"middle\">\n");
}



/*
 * WriteFooter
 *
 * If title is NULL, then it is a meta page!
 */
void
html_page_footer(Page * page, int mode)
{
    if (numfoot)
        html_footnotes();

    svr_puts(server, "</div>\n");
    svr_puts(server, "<div class=\"footer\">\n");

    svr_puts(server, "<i>");
    if (mode == MODE_EDIT) {
	char* title = var_get_val(server->variables, "page");
#if GERMAN
	svr_printf(server, "<b>Sie bearbeiten die Wiki-Seite \'%s\'</b>", title);
#else
	svr_printf(server, "<b>You change the wiki page \'%s\'</b>", title);
#endif
    }
    else if (mode == MODE_ASCII) {
	char* title = var_get_val(server->variables, "page");
#if GERMAN
	svr_printf(server, "<b>Der Quelltext der Wiki-Seite \'%s\'</b>", title);
#else
	svr_printf(server, "<b>The source of the wiki page \'%s\'</b>", title);
#endif
    }
    else {
	char time[MAX_TIMELEN];
	time_t ms = get_time() - request_get_start(server);
	char* owner = page_get_ownername(page);

	page_get_timestring(page, time);
	svr_printf(server, "%s, ", owner);
	svr_printf(server, "%s, ", time);
	svr_printf(server, "%d ms", ms);
#if GERMAN
	if (page_is_edited(page))
	    svr_printf(server,
		       ", <b>wird gerade editiert von %s</b>",
		       page_get_editor(page));
#else
	if (page_is_edited(page))
	    svr_printf(server,
		       ", <b>is edited by %s</b>",
		       page_get_editor(page));
#endif
    }

    svr_puts(server, "</i>\n");

    svr_puts(server, "</div>\n");       /* footer */
    svr_puts(server, "</div>\n");       /* text */
    svr_puts(server, "</body></html>\n");
}


void
html_ParaBegin()
{
    svr_puts(server, "<p>\n");
}

void
html_ParaEnd()
{
    svr_puts(server, "</p>\n");
}

void
html_PreBegin()
{
    svr_puts(server, "<pre>\n");
}

void
html_PreEnd()
{
    svr_puts(server, "</pre>\n");
}

void
html_BlockquoteBegin()
{
    svr_puts(server, "<blockquote>\n");
}

void
html_BlockquoteEnd()
{
    svr_puts(server, "</blockquote>\n");
}

void
html_ruler_begin()
{
    /* as long mozilla does not understand css for hr tag */
    //svr_puts(server, "<hr noshade style=\"color:#8CACBB; size: 1px; background-color: transparent;\">\n");
    svr_puts(server, "<hr noshade size=\"1\" color=\"#8CACBB\">\n");
}

void
html_ruler_end()
{
//    svr_puts(server, "</hr>\n");
}

void
html_ListBegin()
{
    if (indentlevel == 0)
        svr_puts(server, "<ul class=\"level0\">\n");
    else
        svr_puts(server, "<ul>\n");

    indentlevel++;
}

void
html_ListEnd()
{
    indentlevel--;
    if (indentlevel == 0)
        svr_puts(server, "</ul>\n");
    else
        svr_puts(server, "</ul>\n");
}

void
html_NumListBegin()
{
    if (indentlevel == 0)
        svr_puts(server, "<ol class=\"level0\">\n");
    else
        svr_puts(server, "<ol>\n");
    indentlevel++;
}

void
html_NumListEnd()
{
    indentlevel--;
    svr_puts(server, "</ol>\n");
}

void
html_ListItemBegin()
{
    svr_puts(server, "<li>");
    svr_puts(server, "<span class=\"list_item\">");
}

void
html_ListItemEnd()
{
    svr_puts(server, "</span>");
    svr_puts(server, "</li>\n");
}

void
html_LineBegin()
{
}

void
html_LineEnd()
{
    svr_puts(server, "\n");
}

void
html_HeadingBegin(int level)
{
    svr_printf(server, "<h%d>", level);
}

void
html_HeadingEnd(int level)
{
    svr_printf(server, "</h%d>", level);
}

void
html_Footnote(char * note)
{
    numfoot++;
    svr_puts(server, "<sup><a class=\"footnote\" title=\"");
    html_puts(note);
    svr_printf(server, "\" href=\"#%d\">[%d]</a></sup>", numfoot, numfoot);

    footnote[numfoot] = strdup(note);
}

void
html_BoldBegin()
{
    svr_puts(server, "<b>");
}

void
html_BoldEnd()
{
    svr_puts(server, "</b>");
}

void
html_ItalicBegin()
{
    svr_puts(server, "<i>");
}

void
html_ItalicEnd()
{
    svr_puts(server, "</i>");
}

void
html_InternalLink(char * name, char * title, Pagetype type)
{
#if GERMAN
    switch (type) {
    case PT_USER:
	svr_puts(server, "<a class=\"gotopage\" title=\"Homepage\" ");
	break;
    case PT_GROUP:
	svr_puts(server, "<a class=\"gotopage\" title=\"Gruppe\" ");
	break;
    case PT_CATEGORY:
	svr_puts(server, "<a class=\"gotopage\" "
		 "title=\"Kategorie - Aktiviere Filter!\" ");
	break;
    default:
	svr_puts(server, "<a class=\"gotopage\" title=\"Interner Link\" ");
    }
#else
    switch (type) {
    case PT_USER:
	svr_puts(server, "<a class=\"gotopage\" title=\"Homepage\" ");
	break;
    case PT_GROUP:
	svr_puts(server, "<a class=\"gotopage\" title=\"Group\" ");
	break;
    case PT_CATEGORY:
	svr_puts(server, "<a class=\"gotopage\" "
		 "title=\"Category - Add To Filter!\" ");
	break;
    default:
	svr_puts(server, "<a class=\"gotopage\" title=\"Internal Link\" ");
    }
#endif
    svr_printf(server, "href=\"/Wiki/%s\">%s</a>", name, title);
}

void
html_BrokenLink(char * name)
{
    svr_printf(server, "<a class=\"createpage\" ");
#if GERMAN
    svr_puts(server, "title=\"Erzeuge neue Seite...\" ");
#else
    svr_puts(server, "title=\"Create new Page...\" ");
#endif
    svr_printf(server, "href=\"/Wiki/%s\">%s</a>", name, name);
}

void
html_TableBegin(int cells)
{
    svr_puts(server, "<table><tbody>\n");
}

void
html_TableEnd()
{
    svr_puts(server, "</tbody></table>\n");
}

void
html_TableHeadBegin()
{
    svr_puts(server, "<tr>\n");
    tableheader = 1;
}

void
html_TableHeadEnd()
{
    svr_puts(server, "</tr>\n");
    tableheader = 0;
}

void
html_TableRowBegin()
{
    svr_puts(server, "<tr>\n");
}

void
html_TableRowEnd()
{
    svr_puts(server, "</tr>\n");
}

void
html_TableCellBegin()
{
    if (tableheader)
        svr_puts(server, "<th>");
    else
        svr_puts(server, "<td>");
}

void
html_TableCellEnd()
{
    if (tableheader)
        svr_puts(server, "</th>\n");
    else
        svr_puts(server, "</td>\n");
}

void
html_TableNumberBegin()
{
    svr_puts(server, "<td class=\"number\">");
}

void
html_TableNumberEnd()
{
    svr_puts(server, "</td>\n");
}

void
html_url(char * url)
{
    /* is a URL - check protocol */
    if (!memcmp(url, "mailto:", 7)) {
        svr_printf(server, "<a href=\"%s\">%s</a>", url, url + 7);
    }
    else {
        svr_printf(server, "<a href=\"%s\">%s</a>", url, url);
    }
}



void
html_external_link(char * url, char * text)
{
    /* just show extra arrow, if http */
    svr_printf(server,
	       "<a href=\"%s\" title=\"%s\" class=\"gotopage\">%s</a>",
	       url, url, text);
    if (memcmp(url, "http", 4) == 0) {
	svr_printf(server, "<a href=\"%s\" target=\"_blank\">", url);
#if GERMAN
	svr_puts(server,
		 "<img title=\"In neuem Browserfenster...\" alt=\"extern\" "
		 "src=\"/Files/extern.png\"></a>");
#else
	svr_puts(server,
		 "<img title=\"Open in new Browser...\" alt=\"extern\" "
		 "src=\"/Files/extern.png\"></a>");
#endif
    }
}



void
html_image_url(char * url)
{
    svr_printf(server, "<a title=\"%s\" href=\"%s\">", url, url);

    /* is a URL - check protocol */
    if (!memcmp(url, "mailto:", 7)) {
        svr_puts(server, "<img src=\"/Images/mail.png\" alt=\"mail\" >");
    }
    else if (!memcmp(url, "news:", 5)) {
        svr_puts(server, "<img src=\"/Images/news.png\" alt=\"news\" >");
    }
    else if (!memcmp(url, "http:", 5)) {
        svr_puts(server, "<img src=\"/Images/net.png\" alt=\"net\" >");
    }
    else if (!memcmp(url, "file:", 5)) {
        svr_puts(server, "<img src=\"/Images/clip.png\" alt=\"clip\" >");
    }
    else {
        svr_puts(server, "<img src=\"/Images/clip.png\" alt=\"clip\" >");
    }
    svr_puts(server, "</a>");
}

void
html_image(char * image)
{
    svr_printf(server, "<img src=\"/Images/%s.png\" title=\"[%s]\" alt=\"%s\" >", image, image, image);
}



/* pluggable output module for HTML */
Output htm = {
    html_putc,
    html_puts,
    html_page_header,
    html_page_footer,
    html_ParaBegin,
    html_ParaEnd,
    html_PreBegin,
    html_PreEnd,
    html_BlockquoteBegin,
    html_BlockquoteEnd,
    html_ruler_begin,
    html_ruler_end,
    html_ListBegin,
    html_ListEnd,
    html_NumListBegin,
    html_NumListEnd,
    html_ListItemBegin,
    html_ListItemEnd,
    html_LineBegin,
    html_LineEnd,
    html_HeadingBegin,
    html_HeadingEnd,
    html_Footnote,
    html_BoldBegin,
    html_BoldEnd,
    html_ItalicBegin,
    html_ItalicEnd,
    html_InternalLink,
    html_BrokenLink,
    html_TableBegin,
    html_TableEnd,
    html_TableHeadBegin,
    html_TableHeadEnd,
    html_TableRowBegin,
    html_TableRowEnd,
    html_TableCellBegin,
    html_TableCellEnd,
    html_TableNumberBegin,
    html_TableNumberEnd,
    html_image,
    html_url,
    html_image_url,
    html_external_link
};


