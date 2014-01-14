/*
 * page.c - routines to do all the things with our pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */


#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/resource.h>

#define PAGE_PRIVATE

#include "cutewiki.h"
#include "hash.h"
#include "page.h"
#include "page_list.h"
#include "parser.h"
#include "user.h"
#include "misc.h"
#include "rcs.h"



/* static prototypes */
bool page_validate_group(Page*, Page*);
bool page_save_meta(Page * page);


/*
 * page_get_datestring - get date as a (static) string
 */
char*
page_get_datestring (Page * page, char * datestring)
{
    struct tm * stime;

    stime = localtime(&page->time);
    strftime(datestring, MAX_DATELEN-1, "%A, %d. %b. %Y", stime);
    datestring[MAX_DATELEN-1] = '\0';

    return datestring;
}



/*
 * page_get_timestring - get time as a (static) string
 */
char*
page_get_timestring (Page * page, char * timestring)
{
    struct tm *    stime;

    stime = localtime(&page->time);
#ifdef	__OS2__
    strftime(timestring, MAX_TIMELEN-1, "%A,  %d. %b. %Y,  %H:%M", stime);
#else
    strftime(timestring, MAX_TIMELEN-1, "%A,  %d. %b. %Y,  %k:%M", stime);
#endif
    timestring[MAX_TIMELEN-1] = '\0';

    return timestring;
}



/*
 * page_scan_links - find all the links in a document
 *
 * builds a NULL-terminated array
 */

void
page_scan_links(Page* page)
{
    char**	links;
    char*	text;
    size_t   	count;
    size_t     	max;

    if (page == NULL || page->text == NULL)
        return;

    free(page->links);
    page->linkcnt = 0;

    count = 0;
    max = MAX_WIKIWORDS;
    links = calloc(max, sizeof(char*));

    text = page->text;
    while (*text) {
        if (isupper((unsigned char)*text)) {
	    char * word;

	    word = get_alnum(&text);
            if (is_wikiword(word)) {
                size_t i;

                /* is it already in the array? */
                for (i = 0; i < count; i++) {
                    if (!strncmp(links[i], word, MAX_WIKINAME))
                        break;
                }

		/* if the word is new, add it to the array */
                if (i == count) {
                    if (count == max) {
                        size_t i;
                        char**	array;

                        /* copy contents to a new and bigger array */
                        max *= 2;
                        array = calloc(max, sizeof(char*));
                        for (i = 0; i < count; i++) {
                            array[i] = links[i];
                        }
                        free(links);
                        links = array;
                    }
                    links[count++] = word;
                }
            }
            else {
                free(word);
	    }
	}
	else if  (*text == '[') {
            /* step over all text in squares */
	    while (*text && *text != ']' && *text != '\n')
		text++;
	    if (*text == ']')     /* step over bracket */
		text++;
	}
        else
	    text++;
    }

    page->links = links;
    page->linkcnt = count;
}



/*
 *
 */
bool
page_is_category(Page * self)
{
    if (self == NULL)
        return false;

    if (self->pagetype == PT_CATEGORY)
        return true;

    return false;
}



/*
 *
 */
bool
page_is_private(Page * self)
{
    if (self == NULL)
        return false;

    if (self->flags & PF_PRIVATE)
        return true;

    return false;
}



/*
 *
 */
static bool
page_is_mine(Page * self)
{
    if (self == NULL)
        return false;

    if (self->owner) {
	if (!strncmp(self->owner, user_get_logname(), MAX_WIKINAME))
	    return true;
    }

    return false;
}



/*
 * check, if user is member of the pages group(-page)
 */
bool
page_is_member (Page * self)
{
    size_t i;

    if (self == NULL || self->group == NULL)
	return false;

    /* Look for each pages list of links... */
    for (i = 0; i < self->group->linkcnt; i++) {
	if (!strncmp(self->group->links[i], user_get_logname(),
		     MAX_WIKINAME) ) {
	    return true;
	}
    }

    return false;
}



/*
 * check, if the page is visible by user
 */
bool
page_is_seen(Page * self)
{
    if (self == NULL)
        return false;

    if (self->flags & PF_HIDDEN) {
        /* am I a member of the pages group? */
        if (page_is_member(self))
            return true;

        /* is it my page?*/
        if (page_is_mine(self))
            return true;

        return false;
    }

    return true;
}



/*
 * check, if the page is writable by me
 */
bool
page_is_writable(Page * self)
{
    if (self == NULL)
	return false;

    if (self->flags & PF_PRIVATE) {
	/* am I a member of the pages group? */
	if (page_is_member(self))
	    return true;

	/* is it my page?*/
	if (page_is_mine(self))
	    return true;

	return false;
    }

    return true;
}



/*
 *
 */
bool
page_is_hidden(Page * self)
{
    if (self == NULL)
	return true;

    if (self->flags & PF_HIDDEN)
        return true;

    return false;
}



/*
 * page_is_edited - see, if someone other is editing the page
 *
 * After somebody did load the page in the edit form a time
 * starts running. If he did not save the form in between 10
 * minutes, the page is automatically freed again.
 *
 * It gets also feed, if the author does save it.
 */
bool
page_is_edited(Page * self)
{
    if (self == NULL)
	return true;

    if (self->edittime > time(NULL) - 60 * WIKI_EDITTIMEOUT)
        return true;

    return false;
}



/*
 * page_is_saveable - see, if we could save the page
 */
bool
page_is_saveable(Page * self, int seqno)
{
    char * user = user_get_logname();
    char * editor = page_get_editor(self);

    if (self == NULL || user == NULL || editor == NULL)
	return false;

    /* if we are the editor, saving is ok anyway */
    if (strncmp(editor, user, MAX_WIKINAME) == 0)
        return true;

    /* so we are not the last editor! */

    /* stop, if somebody did submit a more actual version */
    if (page_get_seqno(self) > seqno) {
	return false;
    }

    return true;
}



/*
 * Return alternative Title or WikiWord Title
 */
char *
page_get_name(Page * self)
{
    if (self == NULL)
	return NULL;

    return self->name;
}



/*
 * page_get_title - return the title of the page
 */
char *
page_get_title(Page * self)
{
    if (self == NULL)
	return NULL;

    return self->title;
}



/*
 * set the owner of a homepage
 */
static void
page_set_owner(Page * self, const char * owner)
{
    if (self == NULL || owner == NULL)
	return;

    free(self->owner);
    self->owner = NULL;
    if (is_wikiword(owner))
        self->owner = strdup(owner);
}



/*
 * Return the last editor of the page
 */
char *
page_get_owner(Page * self)
{
    if (self != NULL && self->owner)
        return self->owner;

#if GERMAN
    return "UnbekannterAuthor";
#else
    return "UnknownAuthor";
#endif
}



/*
 * Return the name of the last editor of the page
 */
char *
page_get_ownername(Page * self)
{
    if (self != NULL && self->owner) {
	Page * page;

        page = pagelist_find_page(self->owner);
        if (page)
            return page_get_title(page);
    }
#if GERMAN
    return "Unbekannter Author";
#else
    return "Unknown Author";
#endif
}



/*
 * set a topic to a homepage
 */
static void
page_set_topic(Page * self, const char * topic)
{
    if (self == NULL && topic == NULL)
        return;

    free(self->topic);
    self->topic = NULL;
    if (is_wikiword(topic))
        self->topic = strdup( topic );
}



/*
 * page_get-topic - Return the topic of a page
 */
char *
page_get_topic(Page * self)
{
    if (self == NULL)
        return NULL;

    return self->topic;
}



/*
 * page_set_editor - set the editor of the page
 *
 * Also set the time we start editing
 */
void
page_set_editor(Page * self, char * editor)
{
    if (self == NULL || editor == NULL)
        return;

    free(self->editor);
    self->editor = strdup(editor);
    self->edittime = time(NULL);
}



/*
 * Return the actual editor of the page
 */
char *
page_get_editor(Page * self)
{
    if (self != NULL && self->editor)
        return self->editor;

#if GERMAN
    return "UnbekannterEditor";
#else
    return "UnknownEditor";
#endif
}



#if 0
/*
 * Return the actual editor of the page
 */
char *
page_get_editor(Page * self)
{
    if (self != NULL && self->editor) {
	Page * page;

        page = pagelist_find_page(self->editor);
        if (page)
            return page_get_title(page);
    }
#if GERMAN
    return "Unbekannter Editor";
#else
    return "Unknown Editor";
#endif
}
#endif


/*
 * get the name of the group of a pag
 */
char *
page_get_groupname(Page * self)
{
    if (self->group)
        return self->group->name;

    return NULL;
}



/*
 * page_get_type - Get the type of a page
 */
Pagetype
page_get_type(Page * self)
{
    return self->pagetype;
}


/*
 * Get name of file with pagess text in fn
 *
 * This does not mean, that the file does exist!
 */
bool
page_get_textfilename(Page * self, char * fn)
{
    if (self->name) {
        snprintf(fn, MAX_PATH, "%s/%s.wik", pagepath, self->name);
        return true;
    }

    return false;
}



/*
 * Get name of file with meta-information in fn
 *
 * This does not mean, that the file does exist!
 */
bool
page_get_metafilename(Page * self, char * fn)
{
    if (self->name) {
        snprintf(fn, MAX_PATH, "%s/%s.met", pagepath, self->name);
        return true;
    }

    return false;
}



/*
 * Get name of RCS version file
 */
bool
page_get_rcsfilename(Page * self, char * fn)
{
    if (self->name) {
        snprintf(fn, MAX_PATH, "%s/RCS/%s.wik,v", pagepath, self->name);
        return true;
    }

    return false;
}



/*
 *
 */
char *
page_get_text(Page * self)
{
    return self->text;
}



/*
 *
 */
time_t
page_get_time(Page * self)
{
    if (self->time)
        return self->time;

    return 0;
}



/*
 *
 */
int
page_get_seqno(Page * self)
{
    if (self->seqno)
        return self->seqno;

    return 0;
}



/*
 *
 */
bool
page_has_changed(Page * self)
{
    if (self->flags & PF_CHANGED)
        return true;

    return false;
}



Page *
page_new (const char* name, int flags)
{
    Page* self;

    /* allocate memory for new page and set initial values */
    self = malloc(sizeof(Page));
    if (self) {
	self->name = strdup(name);
	self->title = make_spaced_title(name);
	self->text = NULL;
	self->flags = flags;
	self->seqno = 0;
	self->time = 0;
	self->links = NULL;
	self->linkcnt = 0;
	self->owner = NULL;
	self->userid = NULL;
	self->password = NULL;
	self->topic = NULL;
	self->group = NULL;
	self->pagetype = PT_NORMAL;
	self->editor = NULL;
	self->edittime = 0;
    }

    return self;
}



/*
 * remove all files belonging to Page
 */
static bool
page_unlink(Page* page)
{
    char	fn[MAX_PATH];

    page_get_textfilename(page, fn);
    if (unlink(fn))
        return false;

    page_get_metafilename(page, fn);
    if (unlink(fn))
        return false;

    return true;
}

/*
 * remove Page itself just from memory
 */
bool
page_free(Page* page)
{
    free(page->name);
    free(page->text);
    free(page->title);
    free(page->owner);
    free(page->userid);
    free(page->password);
    free(page->topic);
    free(page->editor);
    free(page);

    return true;
}

bool
page_del_force(Page* page)
{
    page_unlink(page);
    page_free(page);
    page = NULL;

    return true;
}



/*
 * Not yet ready!
 */
bool
page_del(Page* page)
{
    /* If page exists at all ... */
    if (!page)
        return false;

    /* remove page's entry from hashtable, if there */
    if ( pagelist_remove_page(page->name) ) {
        Page ** list = pagelist();
        size_t i;

	if (list == NULL)
	    return false;

        /* make shure no other page's group is pointing to us! */
        for (i = 0; list[i] != NULL; i++) {
	    Page * other = list[i];

            if (other->group == page->group) {
                other->group = NULL;
                page_save_meta(other);
            }
	}

        /* now really delete the page itself */
        return page_del_force(page);
    }

    return false;
}



bool
page_input_meta(Page* page, FILE* file)
{
    char buf[1024];
    bool retval = false;

    /* set standards, if no meta info available */
    page->flags &= ~PF_PRIVATE;
    page->flags &= ~PF_HIDDEN;
    page->time = time(NULL);

    if (file) {
        /* read the meta info line by line */
        while( fgets(buf, (int)sizeof(buf), file) ) {
            char key[1024];
            char val[1024];

            sscanf (buf, " %[a-z0-9] : %[^\n]", key, val);

            if (!strncmp(key, "title", 5)) {
                page->title = strdup(val);
            }
            else if (!strncmp(key, "owner", 5)) {
                page->owner = strdup(val);
            }
            else if (!strncmp(key, "time", 4)) {
                page->time = (time_t)atol(val);
            }
#if USERID
	    else if (!strncmp(key, "userid", 6)) {
		user_set_userid(page, strdup(val));
	    }
#endif
            else if (!strncmp(key, "password", 8)) {
                page->password = strdup(val);
		page->pagetype = PT_USER;
            }
            else if (!strncmp(key, "topic", 5)) {
                page->topic = strdup(val);
            }
            else if (!strncmp(key, "group", 5)) {
                page->group = pagelist_find_page(val);
            }
            else if (!strncmp(key, "pagetype", 8)) {
                if (!strncmp(val, "homepage", 8))
                    page->pagetype = PT_USER;
                else if (!strncmp(val, "grouppage", 9))
                    page->pagetype = PT_GROUP;
                else if (!strncmp(val, "category", 8))
                    page->pagetype = PT_CATEGORY;
                else
                    page->pagetype = PT_NORMAL;
            }
            else if (!strncmp(key, "private", 7)) {
                if (!strncmp(val, "yes", 3))
                    page->flags |= PF_PRIVATE;
            }
            else if (!strncmp(key, "hidden", 6)) {
                if (!strncmp(val, "yes", 3))
                    page->flags |= PF_HIDDEN;
            }
        }
        retval = true;
    }

    if (page->title == NULL)
        page->title = make_spaced_title(page->name);

    if (page->owner == NULL) {
#if GERMAN
        page->owner = strdup("UnbekannterAuthor");
#else
        page->owner = strdup("UnknownAuthor");
#endif
    }

    return retval;
}


bool
page_load_meta(Page* page)
{
    char	fn[MAX_PATH];
    FILE *file;
    bool retval = false;

    page_get_metafilename(page, fn);

    /* FIXME: This can be done better... */
    file = fopen(fn, "r");
    if (file) {
        page_input_meta(page, file);
        fclose(file);
        retval = true;
    }

    return retval;
}

/*
 * write meta-info to file
 */
void
page_print_meta(Page* page)
{
    /* now write the meta info */
    svr_printf(server, "title: %s\n", page->title );
    switch (page->pagetype) {
    case PT_USER:
        svr_printf(server, "pagetype: homepage\n");
        break;
    case PT_GROUP:
        svr_printf(server, "pagetype: grouppage\n");
        break;
    default:
        break;
    }
    svr_printf(server, "owner: %s\n", page->owner );
#if USERID
    if (page->userid)
	svr_printf(server, "userid: %s\n", page->userid );
#endif
    if (page->password)
        svr_printf(server, "password: %s\n", page->password );
    if (page->group)
        svr_printf(server, "group: %s\n", page_get_groupname(page) );

    svr_printf(server, "private: %s\n", (page->flags & PF_PRIVATE) ? "yes":"no" );
    svr_printf(server, "hidden: %s\n", (page->flags & PF_HIDDEN) ? "yes":"no" );

    svr_printf(server, "time: %li\n", page->time );

    return;
}

/*
 * write meta-info to file
 */
bool
page_output_meta(Page * page, FILE* file)
{
    if (!file)
        return false;

    /* now write the meta info */
    fprintf(file, "title: %s\n", page->title );
    switch (page->pagetype) {
    case PT_USER:
        fprintf(file, "pagetype: homepage\n");
        break;
    case PT_GROUP:
        fprintf(file, "pagetype: grouppage\n");
        break;
    case PT_CATEGORY:
        fprintf(file, "pagetype: category\n");
        break;
    default:
        break;
    }
    fprintf(file, "owner: %s\n", page->owner );
#if USERID
    if (page->userid)
	fprintf(file, "userid: %s\n", page->userid );
#endif
    if (page->password)
        fprintf(file, "password: %s\n", page->password );
    if (page->group)
        fprintf(file, "group: %s\n", page_get_groupname(page) );
    if (page->topic)
        fprintf(file, "topic: %s\n", page->topic );

    fprintf(file, "private: %s\n", (page->flags & PF_PRIVATE) ? "yes":"no" );
    fprintf(file, "hidden: %s\n", (page->flags & PF_HIDDEN) ? "yes":"no" );

    fprintf(file, "time: %li\n", (long int)page->time );

    return true;
}

/*
 * write meta-info to disk
 */
bool
page_save_meta(Page * page)
{
    FILE*	file;
    char	fn[MAX_PATH];

    /* save page's meta information */
    page_get_metafilename(page, fn);
    file = fopen(fn, "w");
    if (!file)
        return false;

    page_output_meta(page, file);
    fclose(file);

    return true;
}



/*
 * page_load_text - load the text of a page from file
 *
 * set loaded flag, if we did load the text in this function. We later
 * need this to prevent a page's text beeing freed, while the page
 * itself is beeing displayed at that moment.
 */
bool
page_load_text(Page* page, bool* loaded)
{
    char filename[MAX_PATH];
    FILE *file;
    size_t len;

    /* is page already loaded? */
    if (page->text) {
        *loaded = false;
	return true;
    }
    *loaded = true;

    /* load page's text */
    page_get_textfilename(page, filename);
    file = fopen(filename, "r");
    if (file != NULL) {
	fseek(file, 0, SEEK_END);
	len = (size_t)ftell(file);
	if (len > 0) {
	    page->text = (char *)malloc(len + 1);
	    if (page->text == NULL) {
		fclose(file);
		return false;
	    }
	    fseek(file, 0, SEEK_SET);
	    len = fread(page->text, 1, len, file);
	    if (len == 0) {
		free(page->text);
                page->text = NULL;
		fclose(file);
		return false;
	    }
	    page->text[len] = '\0';
	    fclose(file);
	    return true;
	}
	fclose(file);
    }

    return false;
}



/*
 * page_unload_text - save the text of a page to file
 *
 * if loaded was not set, we leave the text in memory as it was.
 *
 * This is needed for example, when we do load the text of the
 * index page to search in itself, while the index page still
 * goes on to be displayed. If we would free it in between,
 * we would loose the rest of it and get nice memory garbage.
 */
bool
page_unload_text(Page * page, bool loaded)
{
    
    if (page->text && loaded) {
	if (page_has_changed(page)) {
	    FILE*	file;
	    char	filename[MAX_PATH];

	    /* now save page's text */
	    page_get_textfilename(page, filename);

	    file = fopen(filename, "w");
	    if (!file)
		return false;

	    page->flags &= ~PF_CHANGED;
	    page->time = time(NULL);

	    fwrite(page->text, 1, strlen(page->text), file);
	    fclose(file);

	    /* now update info about reverse links */
	    page_scan_links(page);

	}
	free(page->text);
	page->text = NULL;
    }

    return true;
}



/*
 * page_print_page
 */
void
page_print_page(Page* page)
{
    bool loaded;

    page_load_text(page, &loaded);
    svr_puts(server, page_get_text(page) );
    page_unload_text(page, loaded);
}

/*
 * if page belongs to group, then unset group
 */
bool
page_validate_group(Page * self, Page * group)
{
    if ( self->group == group ) {
        self->group = NULL;
        page_save_meta(self);
        return true;
    }

    return false;
}



bool
page_edit(const char* name, const char* title, const char* text,
	  const char * topic, const char* owner, const char* userid,
	  const char* password, const char* group, const char * type,
	  int seqno, bool private, bool hidden)
{
    Page * page;

    /* find the page or make one */
    page = pagelist_find_page("name");
    if (page == NULL)
	page = pagelist_insert_page(name, 0);
    else if (!page_is_saveable(page, seqno))
	return false;

    if (title != NULL && strlen(title) > 0) {
	free(page->title);
	page->title = strdup(title);
    }

    if (text != NULL && strlen(text) > 0) {
	free(page->text);
	page->text = strdup(text);
    }

    /* first save the real text */
    page->flags |= PF_CHANGED;
    page_unload_text(page, true);       /* force unloading */

    /* if pagetype was choosen set and save some meta info */
    if (type != NULL) {
	if ( !strncmp(type, "Homepage", 8) ) {
	    page->pagetype = PT_USER;
	    user_set_password(page, "wikiwiki");
	}
	else if ( !strncmp(type, "Grouppage", 9) ) {
	    page->pagetype = PT_GROUP;
	}
	else if ( !strncmp(type, "Category", 8) ) {
	    page->pagetype = PT_CATEGORY;
	}
    }
    page_set_topic(page, topic);
    page_set_owner(page, owner);

    if (page->pagetype == PT_USER) {
#if USERID
	if (userid != NULL && strlen(userid) > 0) {
	    user_set_userid(page, userid);
	}
#endif
	if (password && (strlen(password) > 0) ) {
	    user_set_own_password(page, password);
	}
    }

    if (private)
        page->flags |= PF_PRIVATE;
    else
        page->flags &= ~PF_PRIVATE;

    if (hidden)
        page->flags |= PF_HIDDEN;
    else
        page->flags &= ~PF_HIDDEN;

    page->group = pagelist_find_page(group);
    page->seqno++;
    page->edittime = 0; /* 1970 = give it free for editing */

    page_save_meta(page);

    /* update RCS revision for this page, if text did change */
    if (rcs_available())
	rcs_checkin(page->name, page->owner);

    return true;
}



/*
 * Find the alternative Title (the real term) for a WikiWord
 */
char *
page_find_title (const char* name)
{
    if (name != NULL) {
        Page * page = pagelist_find_page(name);
        if (page)
            return page_get_title(page);
    }
    return "";
}
