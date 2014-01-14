/*
 * menu.c - Web Userinterface
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
#include "user.h"
#include "misc.h"
#include "rcs.h"



static void
write_category(const char * name)
{
    char* category;

    /* Handle the hidden category setting */
    category = var_get_val(server->variables, "cutewiki-category");
    svr_puts(server, "<input type=\"hidden\" name=\"category\" value=\"");
    xml_puts(category);
    svr_puts(server, "\">\n");

    /* Show category button */
    if (category != NULL) {
	svr_printf(server, "<a href=\"/Wiki/CategoryPage\" title=\"");
	xml_puts(category);
	//xml_puts(page_find_title(category));
	svr_printf(server, "\"><img src=\"/Files/filter_add.png\" alt=\"Filter\"></a>\n");

	svr_printf(server, "<a href=\"/FilterOff/%s\" ", name);
#if GERMAN
	svr_printf(server, "title=\"Filter Aus\"><img src=\"/Files/filter_off.png\" alt=\"Filter aus\"></a>\n");
#else
	svr_printf(server, "title=\"Filter off\"><img src=\"/Files/filter_off.png\" alt=\"Filter off\"></a>\n");
#endif
    }
    else {
	svr_printf(server,
#if GERMAN
		   "<a href=\"/Wiki/CategoryPage\" title=\"Kein Filter aktiv!\">"
#else
		   "<a href=\"/Wiki/CategoryPage\" title=\"No filter active!\">"
#endif
		   "<img src=\"/Files/filter_on.png\" alt=\"Filter\"></a>\n"
		  );
    }
}



static void
write_search()
{
    char* searchstring;

    /* Handle search form data */
    searchstring = var_get_val(server->variables, "cutewiki-search");
    svr_printf(server, "<input type=\"text\" size=\"30\" name=\"cutewiki-search\" value=\"");
    xml_puts(searchstring);

    /* set cookie and save criterion for next search */
    svr_set_cookie(server, "cutewiki-search", searchstring);

#if GERMAN
    svr_puts(server, "\" title=\"Geben Sie hier den Suchbegriff ein!\" alt=\"search\">\n");
    svr_puts(server,
	     "<input class=\"bar\" title=\"Titelsuche\" "
	     "type=\"submit\" name=\"titlesearch\" value=\" Titel \" >\n");
    svr_puts(server,
	     "<input class=\"bar\" title=\"Volltextsuche\" "
	     "type=\"submit\" name=\"fullsearch\" value=\" Voll \" >\n");
#else
    svr_puts(server, "\" alt=\"Insert your search string here!\">\n");
    svr_puts(server,
	     "<input class=\"bar\" title=\"Title Search\" "
	     "type=\"submit\" name=\"titlesearch\" value=\" Title \" >\n");
    svr_puts(server,
	     "<input class=\"bar\" title=\"Fulltext Search\" "
	     "type=\"submit\" name=\"fullsearch\" value=\" Full \" >\n");
#endif
}



/*
 * write_user - print link to logged in User and logoff button
 */
static void
write_user()
{
    char * user = user_get_logname();
    char * title = page_find_title(user);

    svr_printf(server, "<a href=\"/Wiki/%s\" title=\"Homepage von %s\">",
	       user, title);
    svr_printf(server, "%s</a>", title);
    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");
    svr_puts(server, "<a href=\"/Wiki/StartPage?logoff=yes\" "
	     "title=\"Logoff\" ><img src=\"/Files/logoff.png\" "
	     "alt=\"Logoff\"></a>\n");

}

void
menu_write_bar(const char * name, int mode )
{
    Page * page;

    svr_printf(server,
	       "<form method=\"post\" accept-charset=\"UTF-8\" "
	       "action=\"/Search/Result\">\n");

    page = pagelist_find_page(name);

    svr_use_utf8(true);
    svr_printf(server, "<div class=\"bar\">\n");

    svr_printf(server,
	       "<a href=\"/Wiki/StartPage\" title=\"Wiki Home\" >"
	       "<img src=\"/Files/home.png\" alt=\"Wiki Home\"></a>\n");
    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");

    /* draw buttons on the bar */
    if (mode != MODE_EDIT && page_is_writable(page)) {
	svr_printf(server,
		   "<a href=\"/Edit/%s\"  title=\"Edit\">"
		   "<img src=\"/Files/edit.png\" alt=\"Edit\"></a>\n", name);
    }

    /* Text output */
    if (mode != MODE_ASCII) {
	svr_printf(server,
		   "<a href=\"/Text/%s\" title=\"Wiki-Source\" "
		   "type=\"text/plain\">"
		   "<img src=\"/Files/txt-file.png\" alt=\"Text\"></a>\n", name);
    }

    if (rcs_available())
	svr_printf(server,
		   "<a href=\"/History/%s\" title=\"Page-History\">"
		   "<img src=\"/Files/version.png\" alt=\"History\"></a>\n", name);

    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");

    svr_printf(server,
	       "<a href=\"/Wiki/IndexPage\" title=\"Index\">"
	       "<img src=\"/Files/index.png\" alt=\"Index\"></a>\n");
    svr_printf(server,
	       "<a href=\"/Wiki/ChangesPage\" title=\"Änderungen\">"
	       "<img src=\"/Files/changes.png\" alt=\"Changes\"></a>\n");
    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");
    svr_printf(server,
	       "<a href=\"/Wiki/InfoPage\" title=\"Wiki-Status\">"
	       "<img src=\"/Files/info.png\" alt=\"Info\"></a>\n");
    svr_printf(server,
	       "<a href=\"/Wiki/HelpPage\" title=\"Hilfe\" target=\"_blank\">"
	       "<img src=\"/Files/help.png\" alt=\"Help\"></a>\n");
    if (mode != MODE_EDIT) {
	svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");

        /* HTML Printer  output */
	svr_printf(server,
		   "<a href=\"/Print/%s\" title=\"Drucken\" "
		   "type=\"text/html\" target=\"_blank\">"
		   "<img src=\"/Files/print.png\" alt=\"Print\"></a>\n", name);

        /* RTF output */
	svr_printf(server,
		   "<a href=\"/Richtext/%s.rtf\" title=\"Richtext\" "
		   "type=\"text/rtf\" target=\"_blank\">"
		   "<img src=\"/Files/rtf-file.png\" alt=\"Richtext\"></a>\n", name);
    }
    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");

    write_category(name);
    write_search();
    svr_puts(server, "<img src=\"/Files/tab.png\" alt=\"|\">");
    write_user();
    svr_puts(server, "</div>\n");
    svr_puts(server, "</form>\n");
}
