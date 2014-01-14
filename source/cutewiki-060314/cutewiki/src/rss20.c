/*
 * rss20.c - provide an RSS 2.0 feed for special readers
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "cutewiki.h"
#include "rss20.h"
#include "time.h"
#include "parser.h"
#include "page_list.h"
#include "misc.h"


/*
 * rss_print_headlines - limit text to one paragraph only
 */
static void
rss_print_headlines(Page * page)
{
    char * dst;
    bool loaded;

    page_load_text(page, &loaded);
    dst = page_get_text(page);
    while (*dst) {
	if (*dst == '\r' || *dst == '\n')
	    break;
	else
	    xml_putc(*dst);
        dst++;
    }
    page_unload_text(page, loaded);
}


/*
 * print the item information for each wiki page
 */
static void
rss_print_item(Page * page, char * wikiurl)
{
    char * topic ;

    svr_puts(server, "    <item>\n");
    svr_printf(server, "      <title>%s</title>\n",
	       page_get_title(page));
    svr_printf(server, "      <link>%s/Wiki/%s.htm</link>\n",
	       wikiurl, page_get_name(page));

    /* print the page's first paragraph only */
    svr_printf(server, "      <description>");
    rss_print_headlines(page);
    svr_printf(server, "      </description>\n");

    svr_printf(server, "      <author>%s</author>\n",
	       page_get_owner(page));
    svr_printf(server, "      <guid>%s/Wiki/%s.htm</guid>\n",
	       wikiurl, page_get_name(page));


    topic = page_get_topic(page);
    if (topic)
	svr_printf(server, "      <category>%s</category>\n", topic);
    svr_puts(server, "    </item>\n");
}



void
rss_handle_feed(char * name)
{
    char wikiurl[256];
    Page** list = pagelist_time_sorted();
    int i;

    sprintf(wikiurl, "http://%s:%d", wiki_get_hostname(), wiki_get_port());

    svr_set_contenttype(server, "application/xml");

    /* print the RSS header */
    svr_use_utf8(true);
    svr_puts(server,
	     "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	     "<rss version=\"2.0\">\n");

    /* print the channel info */
    svr_puts(server, "  <channel>\n");
    svr_printf(server, "    <title>%s</title>\n", wiki_get_wikiname());
    svr_printf(server, "    <link>%s</link>\n", wikiurl);
    svr_printf(server, "    <description>%s</description>\n", wiki_get_description());
    svr_puts(server, "    <generator>CuteWiki</generator>\n");
    svr_puts(server, "    <language>de</language>\n");

    if (list != NULL) {
	/* now loop through the wiki pages */
	for (i = 0; list[i] != NULL; i++) {
	    time_t t;
	    Page * page = list[i];

	    /* if page is younger than a day */
	    if (page_get_time(page) > (time(&t) - 43200)
		&& !page_is_hidden(page)) {
		rss_print_item(page, wikiurl);
	    }
	}
    }

    /* end the channel an all */
    svr_puts(server, "  </channel>\n");
    svr_puts(server, "</rss>\n");
}
