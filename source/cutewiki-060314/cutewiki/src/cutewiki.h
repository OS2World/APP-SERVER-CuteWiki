/*
 * cutewiki.h - All the commonly used things
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef CUTEWIKI_H
#define CUTEWIKI_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <pwd.h>

#include "config.h"
#include "page.h"
#include "http.h"
#include "svr.h"
#include "cfg.h"



#define MAX_PATH        256
#define MAX_WIKINAME    256
#define MAX_WIKIWORDS   32
#define MAX_PARALEN     256
#define MAX_TEXTLEN     1024*1024
#define MAX_DATELEN     64
#define MAX_TIMELEN     128


extern httpd	*server;        /* this is our webserver instance */

/*
 * Infos about one Image file
 */
typedef struct ImageFile ImageFile;
struct ImageFile
{
    char*	mFileName;	/* name of file */
    ImageFile*	mNext;		/* next file */
    int		mUseCnt;	/* number of usages in the pages */
};

/*
 * Infos about one Image file
 */
typedef struct Request Request;
struct Request
{
    char        password[32];
    char	username[32];
    char	realname[64];
};



extern struct Wiki * wiki;

/*
 * External declarations
 */
extern const char gIconName[];
extern uid_t uid;              /* actual user id */

/*
 * Prototypes
 */

char *          wiki_get_wikiname();
char *          wiki_get_description();
char *          wiki_get_hostname();
char *          wiki_get_replhost();
char *          wiki_get_sysname();
char *          wiki_get_release();
char *          wiki_get_machine();
char *		wiki_get_imagedir();
char *		wiki_get_filedir();
int             wiki_get_port();
int             wiki_get_replport();
char *		wiki_get_wordsdir();
Config *        wiki_get_config();
time_t		wiki_get_starttime();



int		wiki_get_calls();




#endif
