/*
 * page_list.c - all things done with lists of pages
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <dirent.h>
#include <fnmatch.h>
#include <string.h>

#define PAGE_PRIVATE

#include "cutewiki.h"
#include "hash.h"
#include "page.h"
#include "page_list.h"
#include "create.h"
#include "parser.h"
#include "misc.h"


/* Variable for all Pages */
static Hash * pagetab;
char* pagepath;



static int
compare_title(const Page** pp1, const Page** pp2)
{
    return strcasecmp((*pp1)->title, (*pp2)->title);
}



static int
compare_time(const Page** pp1, const Page** pp2)
{
    time_t	t1;
    time_t	t2;

    t1 = (*pp1)->time;
    t2 = (*pp2)->time;

    if (t1 < t2)
        return +1;
    if (t1 > t2)
        return -1;

    return 0;
}



static bool found (const char* search, const char* text);
/*
 * match_wild - match with wildcard *
 *
 * This may be called recursively, then stopping at end of line.
 * Not using the initial match() recursively gives speed, if the
 * wildcard * was not used - which is mostly the case.
 */
static bool
match_wild (const char* search, const char* text)
{
    while (*text && *text != '\n') {
	if (found(search, text++))
	    return true;
    }

    return false;
}



/*
 * search_wild - do a wildcard match
 */
static bool
found (const char* search, const char* text)
{
    while (*search) {
	if (*search == '*')
	    return match_wild(search + 1, text);

	if (*search != '?' && toupper(*search) != toupper(*text))
	    return false;

	if (*text == '\n')
	    return false;

	search++;
	text++;
    }

    return true;
}



/*
 * match - does the search argument match the given string?
 */
static bool
match (const char* search, const char* text)
{
    while (*text) {
	if (found(search, text++))
	    return true;
    }

    return false;
}



/*
 * sort_alpha - terminate and sort a list, if entries there
 */
static Page**
sort_alpha (Page ** list, size_t cnt)
{
    if (list == NULL)
	return NULL;

    if (cnt > 0) {
	qsort(list, cnt, sizeof(Page*), (CompFunc)compare_title);
	list[cnt] = NULL;   /* terminate the list */
	return list;
    }

    /* if no entries, free list and return NULL */
    free(list);

    return NULL;
}



/*
 * page_insert - get or create the named page
 */
Page *
pagelist_insert_page (const char* name, int flags)
{
    if (is_wikiword(name)) {
	Page* page;

        /* create the page, if not already there */
	page = (Page*)hash_find(pagetab, name);
	if (page == NULL) {
	    page = page_new(name, flags);
	    if (page)
		/* insert the new page into the page table */
		hash_insert(pagetab, page->name, page);
	}
        return page;
    }

    return NULL;
}



/*
 * page_remove - remove a page from the hash
 */
bool
pagelist_remove_page (const char* name)
{
    return hash_remove(pagetab, name);
}



Page*
pagelist_find_page (const char* name)
{
    if (!is_wikiword(name))
        return NULL;

    return (Page *)hash_find(pagetab, name);
}



/*
 * pagelist - get an array of page pointers
 *
 * needs to be freed after usage!
 */
Page**
pagelist()
{
    return (Page **)hash_get_list(pagetab);
}



Page**
pagelist_alpha_sorted()
{
    Page** list = pagelist();

    if (list == NULL)
	return NULL;

    qsort(list, (size_t)pagelist_get_count(), sizeof(void*), (CompFunc)compare_title);

    return list;
}



Page**
pagelist_time_sorted()
{
    Page** list = pagelist();

    if (list == NULL)
	return NULL;

    qsort(list, (size_t)pagelist_get_count(), sizeof(void*), (CompFunc)compare_time);

    return list;
}



/*
 * page_get_users - get a list of all registered Users
 */
Page**
pagelist_get_users ()
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
        if (list[i]->pagetype == PT_USER)
            list[cnt++] = list[i];
    }

    return sort_alpha(list, cnt);
}



/*
 * page_get_groups - get a list of all groups
 */
Page**
pagelist_get_groups ()
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
	if (list[i]->pagetype == PT_GROUP)
            list[cnt++] = list[i];
    }
    return sort_alpha(list, cnt);
}



/*
 * pagelist_get_categories - get a list of all categories
 */
Page**
pagelist_get_categories ()
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
	if (list[i]->pagetype == PT_CATEGORY)
            list[cnt++] = list[i];
    }
    return sort_alpha(list, cnt);
}



/*
 * page_is_linked - See, if another page has a link to us
 */
static Page *
page_is_linked(Page * self, const char * name)
{
    size_t i;

    if (self == NULL)
	return NULL;

    /* Look for each pages list of links... */
    for (i = 0; i < self->linkcnt; i++) {
	if (!strncmp(self->links[i], name, MAX_WIKINAME)) {
            return self;
        }
    }
    return NULL;
}

Page**
pagelist_of_reverse_links(const char* name)
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
        Page* found = page_is_linked(list[i], name);
        if (found != NULL) {
            list[cnt++] = found;
        }
    }

    return sort_alpha(list, cnt);
}



#if 0
/*
 * pagelist_of_own_pages
 *
 * Get a list of the actual users pages
 */
static Page**
pagelist_of_own_pages(const char* name)
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL || name == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
        Page* found = page_is_mine(list[i]);
        if (found != NULL) {
            list[cnt++] = found;
        }
    }

    return sort_alpha(list, cnt);
}



/*
 * pagelist_of_unused_wikiwords - A user's unused wikiwords
 */
Page**
pagelist_of_unused_wikiwords(const char* name)
{
    Page** list = pagelist_of_own_pages();
    size_t i;
    size_t cnt;

    if (name == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
    }
    return sort_alpha(list, cnt);
}
#endif


/*
 * get_next_category - get the next (hopefully) category string
 */
bool
get_next_category (char* category, const char** text)
{
    const char* ch = *text;
    bool found = false;
    size_t len = MAX_WIKINAME;

    if (text == NULL)
        return false;

    /* eat spaces */
    while (*ch == ' ' || *ch == '\t')
	ch++;

    /* copy the category */
    while (isalnum((unsigned char)*ch) && --len ) {
	*category++ = *ch++;
	found = true;           /* at least on character found */
    }
    *category = '\0';

    /* eat spaces */
    while (*ch == ' ' || *ch == '\t')
	ch++;

    /* eat + and &, which are valid category seperators */
    while (*ch == '+' || *ch == '\t')
	ch++;

    *text = ch;

    return found;
}



/*
 * pagelist_in_category - get a list of pages in all given categories
 *
 * We search for every valid WikiWord, if Tagged as Category or not.
 * This is just for speed. It's up to the user to do it right
 */
Page**
pagelist_in_category(const char* categories)
{
    char category[MAX_WIKINAME];
    size_t i;
    Page** list = pagelist();
    size_t cnt = 0;

    if (list == NULL)
	return NULL;

    /* loop through all the categories given */
    while (get_next_category(category, &categories)) {
        /* for each category get just the pages which are in it */
	for (i = 0, cnt = 0; list[i] != NULL; i++) {
	    Page* found = page_is_linked(list[i], category);
	    if (found) {
		list[cnt++] = found;
	    }
	}
	list[cnt] = NULL;           /* make list smaller */
    }
    
    return sort_alpha(list, cnt);
}



Page**
pagelist_search_topic (const char* search)
{
    Page** list = pagelist();
    size_t i;
    size_t cnt;

    if (list == NULL)
	return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
	/* beware of the pages with NULL topic! */
	Page * page = list[i];

	if (page->topic != NULL && match(search, page->topic))
	    list[cnt++] = page;
    }

    return sort_alpha(list, cnt);
}



/*
 * pagelist_search_title - Search for a title
 *
 * The search can optionally be done with a filter
 */
Page**
pagelist_search_title (const char* search, const char * filter)
{
    Page** list;
    size_t i;
    size_t cnt;

    if (filter == NULL)
	list = pagelist();
    else
	list = pagelist_in_category(filter);
    
    if (list == NULL || search == NULL)
        return NULL;

    for (i = 0, cnt = 0; list[i] != NULL; i++) {
	/* beware of the pages with NULL topic! */
	Page * page = list[i];

	if (page->title != NULL && match(search, page->title))
	    list[cnt++] = page;
    }

    return sort_alpha(list, cnt);
}



/*
 * pagelist_search_full - Fulltext search
 *
 * The search can optionally be done with a filter
 */
Page**
pagelist_search_full(const char* search, const char * filter)
{
    Page** list;
    size_t i;
    size_t cnt;

    if (filter == NULL)
	list = pagelist();
    else
	list = pagelist_in_category(filter);

    if (list == NULL || search == NULL)
        return NULL;

    /* shrink list to the entries we did search for */
    for (i = 0, cnt = 0; list[i] != NULL; i++) {
	Page * page = list[i];
	bool loaded;

	if (page_load_text(page, &loaded) && match(search, page->text))
	    list[cnt++] = page;
	page_unload_text(page, loaded);
    }

    return sort_alpha(list, cnt);
}



/*
 * get the number of pages
 */
size_t
pagelist_get_count()
{
    return hash_get_size(pagetab);
}



/*
 * WriteMainMemory - show used RAM in Kb
 */
size_t
pagelist_get_usedmemory()
{
    Page ** list = pagelist();
    size_t mainmem = 0;
    size_t i;

    if (list == NULL)
        return mainmem;

    /* Loop through first two chars of page name */
    for (i = 0; list[i] != NULL; i++) {
        /* Loop through all the pages beginning with these two chars */
        Page * page = list[i];

        /* calculate mem for page, title and list of links */
        mainmem += sizeof(Page*);
        mainmem += sizeof(Page);
        mainmem += strlen(page->name);
        mainmem += strlen(page->title);
        if (page->owner != NULL)
            mainmem += strlen(page->owner);
        if (page->password != NULL)
            mainmem += strlen(page->password);
        mainmem += page->linkcnt * sizeof(char*) * 32;
    }
    free(list);

    return (mainmem / 1024);
}



size_t
pagelist_get_useddisk()
{
    Page ** list = pagelist();
    size_t diskmem = 0;
    size_t i;

    if (list == NULL)
        return 0;

    /* Loop through all the pages  */
    for (i = 0; list[i] != NULL; i++) {
        Page * page = list[i];
        char filename[MAX_PATH];
        struct stat fstat;

        /* Get size information from file entries in directory */
        page_get_textfilename(page, filename);
        if (!stat(filename, &fstat))
            diskmem += fstat.st_size;

        page_get_metafilename(page, filename);
        if (!stat(filename, &fstat))
            diskmem += fstat.st_size;
    }
    free(list);

    return (diskmem / 1024);
}



/*
 * load_wiki_file - load a file as page text
 */
static void
load_wiki_file(struct dirent *dirent, char * dirpath)
{
    char filename[MAX_PATH];
    char wikiword[MAX_PATH];
    Page *page;
    bool loaded;

    /* Get time information from file */
    snprintf(filename, MAX_PATH, "%s/%s", dirpath, dirent->d_name);

    /* Make WikiWord from filename */
    strncpy(wikiword, dirent->d_name, MAX_WIKINAME);
    wikiword[strlen(wikiword) - 4] = '\0';

    /* create and process each read page*/
    page = pagelist_insert_page(wikiword, 0);
    page_load_text(page, &loaded);
    page_scan_links(page); /* force, even if page has not changed */
    page_unload_text(page, loaded);
}



/*
 * page_init - initializes structures for holding pages
 */
void
pagelist_init(const char* pathname)
{
    Page ** list;
    size_t i;
    char dirpath[MAX_PATH];
    DIR *dir;
    struct dirent *dirent;
    Page * page;        /* for WikiAdmin check */

    pagepath = strdup(pathname);
    pagetab = hash_new();

    snprintf(dirpath, MAX_PATH, "%s", pathname);
    dir = opendir(dirpath);
    if (dir == NULL) {
        fprintf(stderr, "Can not open pages directory %s !\n", dirpath);
        exit(1);
    }

    /* load text, if it is really a wiki file */
    while ((dirent = readdir(dir)) != NULL) {
        if (fnmatch("*.wik", dirent->d_name, 0) == 0) {
	    load_wiki_file(dirent, dirpath);
        }
    }

    /* load meta, when all needed pages to eventually point to exist */
    list = pagelist();
    for (i = 0; list[i] != NULL; i++) {
        page_load_meta(list[i]);
    }

    /* make shure, that the WikiAdmin page is always there */
    page = pagelist_find_page("WikiAdmin");
    if (page == NULL)
	create_wikiadmin();
}



void
pagelist_exit()
{
    Page ** list = pagelist();
    size_t i;

    /* now delete all pages just from memory */
    for (i = 0; list[i] != NULL; i++) {
        page_free(list[i]);
    }
    free(pagepath);
    pagepath = NULL;
}

