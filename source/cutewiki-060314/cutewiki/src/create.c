/*
 * create.c - create very important pages on demand
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "types.h"
#include "cutewiki.h"
#include "page.h"
#include "page_list.h"



bool
create_editpage()
{
    return page_edit("EditPage", "",
		     "This is the text of '''[PageName]''':\n\n"
		     "[EditForm]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}


bool
create_sourcepage()
{
    return page_edit("SourcePage", "",
		     "This is the text of '''[PageName]''':\n\n"
		     "[PageSource]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}


bool
create_searchpage()
{
    return page_edit("SearchPage", "",
		     "This is our search for '''[SearchText]''':\n\n"
		     "[SearchList]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_reversepage()
{
    return page_edit("ReversePage", "ReverseLookup",
		     "These pages do point to '''[PageName]''':\n\n"
		     "[ReverseList]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_errorpage()
{
    return page_edit("ErrorPage", "",
		     "'''[ErrorMessage]'''\n\n"
		     "[ErrorDescription]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_startpage()
{
    return page_edit("StartPage", "",
		     "This is just an example startpage. "
		     "If you like to change the title or "
		     "have to show it up in another language, "
		     "just edit this page and change it.", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_indexpage()
{
    return page_edit("IndexPage", "",
		     "This is the index of all pages: \n\n"
		     "[PageIndex]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_changespage()
{
    return page_edit("ChangesPage", "",
		     "This is the history of the latest "
		     "changes in our wiki: \n\n"
		     "[RecentChanges]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_infopage()
{
    return page_edit("InfoPage", "",
		     "This is the Status of the Wiki: \n\n"
		     "|| Parameter  | actual Value |\n"
		     "| Page count  | [PageCount]  |\n"
		     "| Memory consumption | [MainMemory] |\n"
		     "| Disk usage  | [DiskUsage]  |", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_deletepage()
{
    return page_edit("DeletePage", "",
		     "You deleted the page '''[PageName]'''! \n\n"
		     "Go on at the StartPage...\n", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_categorypage()
{
    return page_edit("CategoryPage", "",
		     "This is a list of all categories you did define in "
		     "this Wiki:\n\n"
		     "-----\n"
		     "[CategoryList]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_helppage()
{
    return page_edit("HelpPage", "",
		     "This is an initial HelpPage.\n"
		     "-----\n"
		     "The little icons in the menu bar have the "
		     "following meaning:\n\n"
		     " 1. The HomePage - go here to get an overview of this wiki.\n"
		     " 2. The Editor - if visible, click here to "
		     "edit the actual page.\n"
		     " 3. The index - see all pages you are allowed to see.\n"
		     " 4. The latest changes - see, what pages have been changed the last 7 days.\n"
		     " 5. The info - get some infos about the state of this wiki.\n"
		     " 6. This help - well, you already see it here.\n"
		     " 7. Print view - you get a new window with the page in a printable form.\n"
		     " 8. Ascii view - get the page in pure ASCII form.\n"
		     " 9. Richtext view - click here to get your page in Richtxt format.\n"
		     " 10. Search form - insert some text and search it in the page titles or fulltext.\n"
		     " 11. Personal page - click on your name to get to your own page.\n"
		     "-----"
		     "There is much more to tell, especially about "
		     "the formatting options of this wiki. But this "
		     "will come later...", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_histpage()
{
    return page_edit("HistoryPage", "",
		     "This is the history of '''[PageName]''':\n\n"
		     "[PageHistory]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_diffpage()
{
    return page_edit("DiffPage", "",
		     "These are the changes to revision [PageRevision] for page "
		     "'''[PageName]''':\n\n"
		     "[PageDiffs]", "",
		     "WikiAdmin", "", "", "", "",
		     0, true, false);
}



bool
create_wikiadmin()
{
    /* The admin get's a homepage and the default password */
    return page_edit("WikiAdmin", "",
		     "You are the Wiki's administrator. "
		     "Congratulations!\n\n"
		     "-----\n"
		     "This is the user password reset. "
		     "Just type in his name an enter:\n"
		     "[PasswordReset]", "",
		     "WikiAdmin", "wikiadmin", "", "", "Homepage",
		     0, true, false);
}



/*
 * create_special_page - force create a needed page
 *
 * We can not live without a bare minimum of special pages,
 * like the StartPage, the EditPage and so on. This may also
 * be the case, if the wiki is started the first time without
 * any pages at all. So we force create them on demand.
 *
 * It may still be the case, that somebody does change the
 * Edit page in a way, that it can not edit anymore. So,
 * Better we should make these read only for the masses.  :-)
 */
bool
create_special_page(char * name)
{
    if (strcmp(name, "EditPage") == 0) {
	if (create_editpage())
	    return true;
    }
    else if (strcmp(name, "SourcePage") == 0) {
	if (create_sourcepage())
	    return true;
    }
    else if (strcmp(name, "ReversePage") == 0) {
	if (create_reversepage())
	    return true;
    }
    else if (strcmp(name, "SearchPage") == 0) {
	if (create_searchpage())
	    return true;
    }
    else if (strcmp(name, "ErrorPage") == 0) {
	if (create_errorpage())
	    return true;
    }
    else if (strcmp(name, "StartPage") == 0) {
	if (create_startpage())
	    return true;
    }
    else if (strcmp(name, "IndexPage") == 0) {
	if (create_indexpage())
	    return true;
    }
    else if (strcmp(name, "ChangesPage") == 0) {
	if (create_changespage())
	    return true;
    }
    else if (strcmp(name, "InfoPage") == 0) {
	if (create_infopage())
	    return true;
    }
    else if (strcmp(name, "ErrorPage") == 0) {
	if (create_errorpage())
	    return true;
    }
    else if (strcmp(name, "DeletePage") == 0) {
	if (create_deletepage())
	    return true;
    }
    else if (strcmp(name, "CategoryPage") == 0) {
	if (create_categorypage())
	    return true;
    }
    else if (strcmp(name, "HelpPage") == 0) {
	if (create_helppage())
	    return true;
    }
    else if (strcmp(name, "HistoryPage") == 0) {
	if (create_histpage())
	    return true;
    }
    else if (strcmp(name, "DiffPage") == 0) {
	if (create_diffpage())
	    return true;
    }
    else if (strcmp(name, "WikiAdmin") == 0) {
	if (create_wikiadmin())
	    return true;
    }

    return false;
}



/*
 * create_is_special - see, if it is a special page we urgently need
 */
bool
create_is_special(char * name)
{
    if (!strcmp(name, "EditPage"))
	return true;
    else if (!strcmp(name, "SourcePage"))
	return true;
    else if (!strcmp(name, "ReversePage"))
	return true;
    else if (!strcmp(name, "SearchPage"))
	return true;
    else if (!strcmp(name, "ErrorPage"))
	return true;
    else if (!strcmp(name, "StartPage"))
	return true;
    else if (!strcmp(name, "IndexPage"))
	return true;
    else if (!strcmp(name, "ChangesPage"))
	return true;
    else if (!strcmp(name, "InfoPage"))
	return true;
    else if (!strcmp(name, "ErrorPage"))
	return true;
    else if (!strcmp(name, "DeletePage"))
	return true;
    else if (!strcmp(name, "CategoryPage"))
	return true;
    else if (!strcmp(name, "HelpPage"))
	return true;
    else if (!strcmp(name, "WikiAdmin"))
	return true;
    else if (!strcmp(name, "HistoryPage"))
	return true;
    else if (!strcmp(name, "DiffPage"))
	return true;

    return false;
}
