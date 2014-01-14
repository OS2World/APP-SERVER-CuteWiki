/*
 * page.h - routines to do all the things with our pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef PAGE_H
#define PAGE_H

#include "types.h"



/*
 * Infos about one Page
 */
typedef struct Page Page;

typedef enum
{
    PT_NORMAL,  		/* page is a normal wiki page */
    PT_USER,			/* page is a homepage */
    PT_GROUP,			/* page defines a group of users */
    PT_CATEGORY                 /* page defines a category of pages */
} Pagetype;



#ifdef PAGE_PRIVATE

enum PageFlags
{
    PF_CHANGED = 1,		/* page differs from file version */
    PF_PRIVATE = 2,    		/* can not be edited by others */
    PF_HIDDEN = 4    		/* can not be seen by others */
};

struct Page
{
    char*	name;		/* name of page */
    char*	text;		/* page contents in ASCII */
    int		flags;		/* flags */
    int		seqno;		/* sequence number */
    time_t	time;		/* filetime */
    char**	links;		/* list of links */
    size_t      linkcnt;	/* number of links */

    /* meta information */
    Pagetype    pagetype;       /* Normal, Homepage or Grouppage */
    char*	title;        	/* preferred alternative title */
    char*	owner;		/* last editor of page */
    char*	userid;		/* a short unix like username */
    char*	password;	/* crypted password of owner */
    char*	topic;		/* the topic page of a page */
    Page*       group;          /* pointer to group page */

    /* information which is not saved */
    char*       editor;         /* person who loaded an editform */
    time_t	edittime;	/* the time the form was load */
};
#endif



/* Routines for all pages */
Page * 		page_new(const char* name, int flags);
bool		page_del(Page* page);
bool            page_free(Page* page);
char *          page_find_title(const char* name);

/* Page related routines */
char* 		page_get_name(Page * page);
char*		page_get_title(Page * page);
char *          page_get_owner(Page * self);
char *          page_get_ownername(Page * self);
char *          page_get_password(Page * self);
bool		page_check_password (Page * page, char * password);
void		page_set_password(Page * self, const char * password);
char *		page_get_groupname(Page * self);
char* 		page_get_text(Page * page);
time_t		page_get_time(Page * self);
bool            page_get_textfilename(Page * self, char * fn);
bool		page_get_rcsfilename(Page * self, char * fn);
char *          page_get_topic(Page * self);
void            page_set_editor(Page * self, char * editor);
char * 		page_get_editor(Page * self);

char* 		page_get_timestring (Page * page, char *);
char* 		page_get_datestring (Page * page, char *);
Pagetype        page_get_type(Page * self);

int		page_get_seqno(Page * self);
bool		page_is_private(Page * self);
bool		page_is_writable(Page * self);
bool		page_is_hidden(Page * self);
bool		page_is_edited(Page * self);
bool            page_is_saveable(Page * self, int seqno);
bool            page_is_seen(Page * self);
bool		page_is_category(Page * self);

bool		page_has_changed(Page * self);
bool            page_save_meta(Page * page);
bool 		page_edit(const char* name, const char* title,
			  const char* text, const char * topic,
			  const char* owner, const char* userid,
			  const char* password, const char* groupname,
			  const char * pagetype,
			  int seqno, bool private, bool hidden);
bool 		page_load_text(Page * page, bool* loaded);
bool 		page_unload_text(Page* page, bool loaded);
bool		page_load_meta(Page* page);
void		page_scan_links(Page* page);
Pagetype	page_get_pagetype(Page * self);

void		page_print_meta(Page* page);
void		page_print_page(Page* page);

bool		page_get_textfilename(Page * self, char * fn);
bool		page_get_metafilename(Page * self, char * fn);



#endif
