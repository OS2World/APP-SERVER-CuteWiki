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
#include <assert.h>
#include <string.h>

#include "config.h"
#include "cutewiki.h"
#include "svr.h"
#include "parser.h"
#include "misc.h"


/* Avoid passing it all the time */

/* Footnotes */
static int numfoot;                 /* counted number of footnotes */
static char * footnote [100];       /* pointer to footnotes */

static int tableheader = 0;     /* header or normal table row */
static int indentlevel = 0;


/* internal prototypes */
void	print_ruler_begin();

/*
 * print_putc - outputs a character to the server
 */
void
print_putc(char ch)
{
    xml_putc(ch);
}



/*
 * print_puts - outputs a string to the server
 */
void
print_puts(char * str)
{
    xml_puts(str);
}


static void
print_footnotes()
{
    int i;

    print_ruler_begin();

    svr_puts(server, "<div class=\"footnotes\">\n");
    for (i=1; i<=numfoot; i++) {
        svr_printf(server, "<a name=\"%d\">[%d]</a> ", i, i);
        print_puts(footnote[i]);
        free(footnote[i]);
        svr_puts(server, "<br>\n");
    }
    svr_puts(server, "</div>\n");
}


static void
print_InfoLine(char * string)
{
    svr_printf(server, "<script type=\"text/javascript\">\n"
                "<!--\n"
                "window.status = \"%s\"\n"
                "//-->\n"
                "</script>\n", string);
}



void
print_page_header(Page * page, int mode)
{
    numfoot = 0;        /* reset footnote counter */
    char * topic = page_get_topic(page);
    char * topictitle = page_find_title(topic);
//    char * name = page_get_name(page);
    char * title = page_get_title(page);

    svr_use_utf8(true);
    svr_puts(server, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    
    svr_puts(server, "<html>\n\n<head>\n");
    svr_puts(server, "  <link rel=\"stylesheet\" type=\"text/css\" href=\"/Files/cwprint.css\">\n");

    /* special section for going around bugs of IE */
    svr_puts(server, "  <!--[if gte IE 5]>\n");
    svr_puts(server, "  <link rel=\"stylesheet\" type=\"text/css\" href=\"/Files/cwprint_ie.css\">\n");
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
    svr_puts(server, "</head>\n\n");
    svr_puts(server, "<body>\n");

    /* Here the real text begins */
    svr_puts(server, "<div class=\"text\">\n");
    svr_puts(server, "<div class=\"header\">\n");

    print_InfoLine(title);

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
    svr_printf(server, "%s\n", title);

    if (topic && (strlen(topictitle) > 0)) {
        svr_puts(server, "<span class=\"topic\">");
#if GERMAN
        svr_printf(server, "  /  <a href=\"/Print/%s\" title=\"Thema\">%s</a>", topic, topictitle);
#else
        svr_printf(server, "  /  <a href=\"/Print/%s\" title=\"Topic\">%s</a>", topic, topictitle);
#endif
        svr_puts(server, "</span>");
    }

    svr_puts(server, "</div>\n\n");
    svr_puts(server, "<div class=\"middle\">\n");
}

/*
 * WriteFooter
 *
 * If title is NULL, then it is a meta page!
 */
void
print_page_footer(Page * page, int mode)
{
    if (numfoot > 0)
        print_footnotes();

    svr_puts(server, "</div>\n");
    svr_puts(server, "<div class=\"footer\">\n");
    if (page) {
	char time[MAX_TIMELEN];

        page_get_timestring(page, time);
        svr_puts(server, "<i>");
        svr_printf(server, "%s, ", page_get_ownername(page));
        svr_printf(server, "%s", time);
        svr_puts(server, "</i>\n");
    }
    svr_puts(server, "</div></div>\n");
    svr_puts(server, "</body></html>\n");
}


void
print_ParaBegin()
{
    svr_puts(server, "<p>\n");
}

void
print_ParaEnd()
{
    svr_puts(server, "</p>\n");
}

void
print_PreBegin()
{
    svr_puts(server, "<pre>\n");
}

void
print_PreEnd()
{
    svr_puts(server, "</pre>\n");
}

void
print_BlockquoteBegin()
{
    svr_puts(server, "<blockquote>\n");
}

void
print_BlockquoteEnd()
{
    svr_puts(server, "</blockquote>\n");
}

void
print_ruler_begin()
{
    /* as long mozilla does not understand css for hr tag */
    //svr_puts(server, "<hr noshade style=\"color:#8CACBB; size: 1px; background-color: transparent;\">\n");
    svr_puts(server, "<hr noshade size=\"1\" color=\"#8CACBB\">\n");
}

void
print_ruler_end()
{
    //svr_puts(server, "</hr>\n");
}

void
print_ListBegin()
{
    svr_puts(server, "<ul>\n");
    indentlevel++;
}

void
print_ListEnd()
{
    indentlevel--;
    svr_puts(server, "</ul>\n");
}

void
print_NumListBegin()
{
    svr_puts(server, "<ol>\n");
    indentlevel++;
}

void
print_NumListEnd()
{
    indentlevel--;
    svr_puts(server, "</ol>\n");
}

void
print_list_item_begin()
{
    svr_puts(server, "<li>");
    svr_puts(server, "<span class=\"list_item\">");
}

void
print_list_item_end()
{
    svr_puts(server, "</span>");
    svr_puts(server, "</li>\n");
}

void
print_LineBegin()
{
}

void
print_LineEnd()
{
    svr_puts(server, "\n");
}

void
print_HeadingBegin(int level)
{
    svr_printf(server, "<h%d>", level);
}

void
print_HeadingEnd(int level)
{
    svr_printf(server, "</h%d>", level);
}

void
print_footnote(char * note)
{
    numfoot++;
    svr_puts(server, "<sup><a class=\"footnote\" title=\"");
    print_puts(note);
    svr_printf(server, "\" href=\"#%d\">[%d]</a></sup>", numfoot, numfoot);
    footnote[numfoot] = strdup(note);
}

void
print_BoldBegin()
{
    svr_puts(server, "<b>");
}

void
print_BoldEnd()
{
    svr_puts(server, "</b>");
}

void
print_ItalicBegin()
{
    svr_puts(server, "<i>");
}

void
print_ItalicEnd()
{
    svr_puts(server, "</i>");
}

void
print_InternalLink(char * name, char * title, Pagetype type)
{
    svr_printf(server, "<a class=\"gotopage\" href=\"/Print/%s\">%s</a>", name, title);
}

void
print_BrokenLink(char * name)
{
    svr_printf(server, "%s", name);
}

void
print_TableBegin(int cells)
{
    svr_puts(server, "<table><tbody>\n");
}

void
print_TableEnd()
{
    svr_puts(server, "</tbody></table>\n");
}

void
print_TableHeadBegin()
{
    svr_puts(server, "<tr>");
    tableheader = 1;
}

void
print_TableHeadEnd()
{
    svr_puts(server, "</tr>");
    tableheader = 0;
}

void
print_TableRowBegin()
{
    svr_puts(server, "<tr>");
}

void
print_TableRowEnd()
{
    svr_puts(server, "</tr>");
}

void
print_TableCellBegin()
{
    if (tableheader)
        svr_puts(server, "<th>");
    else
        svr_puts(server, "<td>");
}

void
print_TableCellEnd()
{
    if (tableheader)
        svr_puts(server, "</th>");
    else
        svr_puts(server, "</td>");
}

void
print_TableNumberBegin()
{
    svr_puts(server, "<td class=\"number\">");
}

void
print_TableNumberEnd()
{
    svr_puts(server, "</td>");
}



void
print_url(char * url)
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
print_external_link(char * url, char * text)
{
    svr_printf(server, "<a href=\"%s\" class=\"gotopage\">%s</a>", url, text);
    print_footnote(url);
}



void
print_image_url(char * url)
{
    /* is a URL - check protocol */
    svr_printf(server, "<a href=\"%s\">", url);
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
    print_footnote(url);
}

void
print_image(char * image)
{
    svr_printf(server, "<img src=\"/Images/%s.png\" title=\"[%s]\" alt=\"%s\" >", image, image, image);
}



/* pluggable output module for HTML */
Output prt = {
    print_putc,
    print_puts,
    print_page_header,
    print_page_footer,
    print_ParaBegin,
    print_ParaEnd,
    print_PreBegin,
    print_PreEnd,
    print_BlockquoteBegin,
    print_BlockquoteEnd,
    print_ruler_begin,
    print_ruler_end,
    print_ListBegin,
    print_ListEnd,
    print_NumListBegin,
    print_NumListEnd,
    print_list_item_begin,
    print_list_item_end,
    print_LineBegin,
    print_LineEnd,
    print_HeadingBegin,
    print_HeadingEnd,
    print_footnote,
    print_BoldBegin,
    print_BoldEnd,
    print_ItalicBegin,
    print_ItalicEnd,
    print_InternalLink,
    print_BrokenLink,
    print_TableBegin,
    print_TableEnd,
    print_TableHeadBegin,
    print_TableHeadEnd,
    print_TableRowBegin,
    print_TableRowEnd,
    print_TableCellBegin,
    print_TableCellEnd,
    print_TableNumberBegin,
    print_TableNumberEnd,
    print_image,
    print_url,
    print_image_url,
    print_external_link
};


