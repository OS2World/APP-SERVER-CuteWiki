/*
 * svr.c - implementation of a simple webserver
 *
 * Copyright 2003 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef SVR_H
#define SVR_H 1



#include <time.h>
#include <sys/time.h>

#include "http.h"



#define CHILD 0
#define ERROR -1

/* entry handle types */
#define	SVR_HANDLE_FILE		1
#define SVR_HANDLE_C_FUNCT	2
#define SVR_HANDLE_EMBER_FUNCT	3
#define SVR_HANDLE_STATIC	4
#define SVR_HANDLE_WILDCARD	5
#define SVR_HANDLE_C_WILDCARD	6


/* Global variables for wiki settings */
extern httpd	*server;        /* this is our webserver instance */



/* prototypes */
void	svr_init(void);
void	svr_exit(void);
httpd *	svr_new(char *, int);
void	svr_del(httpd *);

int 	svr_register_string (httpd*,char*,char*,int,int(*)(),char*);
int 	svr_register_file (httpd*,char*,char*,int,int(*)(),char*);
int 	svr_register_dir (httpd*,char*,int(*)(),char*);
int 	svr_register_filehandler (httpd*,char*,char*,int,int(*)(),void(*)());
int 	svr_register_dirhandler (httpd*,char*,int(*)(),void(*)());

int	svr_check_auth(httpd * server, char * realm);
void 	svr_force_auth(httpd * server, char * realm);

void 	svr_add_header (httpd*, char*);
void 	svr_set_contenttype (httpd*, char*);
void 	svr_set_cookie (httpd*, char*, char*);
void	svr_expire_cookie(httpd*, char*);
void 	svr_set_response (httpd*, char*);

int 	svr_get_connection (httpd*, struct timeval*);
int 	svr_read_request (httpd*);
void 	svr_process_request (httpd*);
void 	svr_end_request (httpd*);

void	svr_use_utf8 (bool);
void 	svr_puts (httpd*, const char*);
void 	svr_putc (httpd *server, char ch);
void 	svr_printf (httpd*, char*, ...);
char*	svr_encode_url (char *);

int     svr_get_request_start();


httpContent *svr_find_content (httpd*, httpDir*, char*);
httpDir *svr_find_dir (httpd*, char*, int);

void 	svr_send_headers (httpd*);
int 	svr_send_direntry (httpd*, httpContent*, char*);
void 	svr_send_file (httpd*, char*);
void 	svr_send_text (httpd*, char*);
void 	svr_send_static (httpd*, char*);
void 	svr_send_binary(httpd *, char*, int);
void 	svr_send_err304 (httpd*);
void 	svr_send_err403 (httpd*);
void 	svr_send_err404 (httpd*);

void 	svr_set_filebase(httpd*, char*);
void 	svr_set_errorlog(httpd*, FILE*);
void 	svr_set_accesslog(httpd*, FILE*);
void 	svr_write_accesslog (httpd*);
void 	svr_write_errorlog (httpd*, char*, char*);



#endif
