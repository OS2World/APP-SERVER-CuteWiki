/*
 * request.h - webserver's request handling
 *
 * Copyright 2002 Hughes Technologies Pty Ltd.  All rights reserved.
 * Copyright 2003 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "config.h"
#include "svr.h"
#include "http.h"



#ifndef REQUEST_H
#define REQUEST_H

char * 	request_get_methodname(httpd * server);
void 	request_clear(httpd * server);
int 	request_get_method(httpd * server);
char * 	request_get_uri(httpd * server);
int 	request_get_length(httpd * server);
char * 	request_get_type(httpd * server);
int 	request_get_start(httpd * server);
int     request_read(httpd * server, char * req);

#endif
