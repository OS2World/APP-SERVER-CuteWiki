/*
 * http.h - headers of the http protocol part
 *
 * Copyright 2002 Hughes Technologies Pty Ltd.  All rights reserved.
 * Copyright 2003 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef HTTP_H
#define HTTP_H


#include <stdio.h>

#include "types.h"
#include "var.h"


#define	LEVEL_NOTICE	"notice"
#define LEVEL_ERROR	"error"


#define	HTTP_PORT 		80
//#define HTTP_MAX_LEN		10240*10
#define HTTP_MAX_LEN		10240*100
#define HTTP_MAX_URL		1024
#define HTTP_MAX_HEADERS	1024
#define HTTP_MAX_AUTH		128
#define	HTTP_IP_ADDR_LEN	17
#define	HTTP_TIME_STRING_LEN	40
#define	HTTP_READ_BUF_LEN	4096
#define	HTTP_ANY_ADDR		NULL

#define	HTTP_GET		1
#define	HTTP_POST		2

#define	HTTP_TRUE		1
#define HTTP_FALSE		0

#define HTTP_METHOD_ERROR "\n<B>ERROR : Method Not Implemented</B>\n\n"



typedef struct http_content{
    char 	*name;
    int		type;
    int 	indexFlag;
    void 	(*function)();
    char 	*data;
    char  	*path;
    int		(*preload)();
    struct	http_content 	*next;
} httpContent;

typedef struct {
    int	length;
    httpContent	*content;
    bool utf8;
    bool headersSent;
    char headers[HTTP_MAX_HEADERS];
    char response[HTTP_MAX_URL];
    char contentType[HTTP_MAX_URL];
} httpRes;


typedef struct http_dir{
    char	*name;
    struct	http_dir *children;
    struct	http_dir *next;
    struct	http_content *entries;
} httpDir;


typedef	struct {
    int		method;
    int 	contentLength;
    int 	authLength;
    char	path[HTTP_MAX_URL];
    char  	userAgent[HTTP_MAX_URL];
    char   	referer[HTTP_MAX_URL];
    char   	ifModified[HTTP_MAX_URL];
    char   	contentType[HTTP_MAX_URL];
    char   	authUser[HTTP_MAX_AUTH];
    char   	authPassword[HTTP_MAX_AUTH];
    time_t      starttime;
} httpReq;

typedef struct {
    int		port;
    int 	serverSock;
    int 	clientSock;
    int 	readBufRemain;
    int 	startTime;
    char	client_ip[HTTP_IP_ADDR_LEN];
    char 	fileBasePath[HTTP_MAX_URL];
    char 	readBuf[HTTP_READ_BUF_LEN + 1];
    char 	*host;
    char 	*readBufPtr;
    httpReq	request;
    httpRes 	response;
    Var*        variables;
    httpDir	*content;
    FILE	*accessLog;
    FILE	*errorLog;
} httpd;



/* prototypes */
void 	http_to_utf (char *, const char *);
char*	http_escape (char*);

void 	http_send_headers (httpd*, int,int);
void 	http_send_file (httpd*, char*);
void 	http_store_data (httpd*, char*);

int 	http_read_buf (httpd*, char*, int);
int 	http_read_char (httpd*, char*);
int 	http_read_line (httpd*, char*, int);
int 	http_check_modified (httpd*, int);




#endif 
