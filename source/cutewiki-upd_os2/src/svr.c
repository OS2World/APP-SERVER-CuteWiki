/*
 * svr.c - implementation of a simple webserver
 *
 * Copyright 2002 Hughes Technologies Pty Ltd.  All rights reserved.
 * Copyright 2003 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h> 
#include <sys/file.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <stdarg.h>

#include "config.h"
#include "svr.h"
#include "http.h"
#include "request.h"
#include "types.h"

#ifdef	__OS2__
  #define	socklen_t	__socklen_t
#endif



/* Global variables for wiki settings */
httpd	*server;        /* this is our webserver instance */



/*
 * We are getting a deamon
 */
void
svr_init()
{
    int i;
    pid_t pid;

    if((pid = fork()) != CHILD)
        exit(0);
    if(setsid() == ERROR)
        exit(0);

    chdir("/");         /* give free the actual directory */
    umask(0022);        /* rw-r--r-- */

    /* close all open filedescritors */
    for(i=sysconf(_SC_OPEN_MAX); i>0; i--)
        close(i);

}



void
svr_exit()
{
}


/*
 * svr_encode_url - must be freed later.
 */
char *
svr_encode_url(char * str)
{
    char    *new,
    *cp;

    new = (char *)http_escape(str);
    if (new == NULL) {
        return(NULL);
    }

    cp = new;
    while(*cp) {
        if (*cp == ' ')
            *cp = '+';
        cp++;
    }

    return(new);
}

httpd *
svr_new(char * host, int port)
{
    httpd	*new;
    int	sock, opt;
    struct  sockaddr_in     addr;

    /*
     ** Create the handle and setup it's basic config
     */
    new = malloc(sizeof(httpd));
    if (new == NULL)
        return(NULL);
    bzero(new, sizeof(httpd));
    new->port = port;
    if (host == HTTP_ANY_ADDR)
        new->host = HTTP_ANY_ADDR;
    else
        new->host = strdup(host);
    new->content = (httpDir*)malloc(sizeof(httpDir));
    bzero(new->content,sizeof(httpDir));
    new->content->name = strdup("");

    /*
     ** Setup the socket
     */

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock  < 0) {
        free(new);
        return(NULL);
    }
#ifdef SO_REUSEADDR
    opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,sizeof(int));
#endif
    new->serverSock = sock;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    if (new->host == HTTP_ANY_ADDR) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else {
        addr.sin_addr.s_addr = inet_addr(new->host);
    }
    addr.sin_port = htons((u_short)new->port);
    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) <0) {
        close(sock);
        free(new);
        return(NULL);
    }
    listen(sock, 128);
    new->startTime = time(NULL);
    return(new);
}

void
svr_del(httpd * server)
{
    if (server == NULL)
        return;

    if (server->host)
        free(server->host);

    free(server);
    server = NULL;
}



int
svr_get_connection(httpd * server, struct timeval *timeout)
{
    int	result;
    fd_set	fds;
    struct  sockaddr_in     addr;
    socklen_t  addrLen;
    char	*ipaddr;

    FD_ZERO(&fds);
    FD_SET(server->serverSock, &fds);
    result = 0;
    while(result == 0) {
        struct timeval timo;

        timo.tv_sec = timeout->tv_sec;
        timo.tv_usec = timeout->tv_usec;

        result = select(server->serverSock + 1, &fds, 0, 0, &timo);
        if (result < 0)
            return(-1);
        if (timeout != 0 && result == 0)
            return(0);
        if (result > 0)
            break;
    }
    bzero(&addr, sizeof(addr));
    addrLen = sizeof(addr);
    server->clientSock = accept(server->serverSock,(struct sockaddr *)&addr,
                                &addrLen);
    ipaddr = inet_ntoa(addr.sin_addr);
    if (ipaddr)
        strncpy(server->client_ip, ipaddr, HTTP_IP_ADDR_LEN);
    else
        *server->client_ip = 0;
    server->readBufRemain = 0;
    server->readBufPtr = NULL;

    return(1);
}



int
svr_read_request(httpd * server)
{
    static char req[HTTP_MAX_LEN];
    int retval;

    /* Setup for a standard response */
    strcpy(server->response.headers,
           "Server: CuteWiki\n");
    strcpy(server->response.contentType, "text/html");
    strcpy(server->response.response,"200 Output Follows\n");
    server->response.headersSent = false;
    server->response.utf8 = true;

    retval = request_read(server, req);
    if (retval != 0) {
	write(server->clientSock, HTTP_METHOD_ERROR,
	      strlen(HTTP_METHOD_ERROR));
	write(server->clientSock, req, strlen(req));
	svr_write_errorlog(server,LEVEL_ERROR,
			   "Invalid method received");
    }
    return retval;
}



void
svr_process_request(httpd * server)
{
    char	dirName[HTTP_MAX_URL];
    char 	entryName[HTTP_MAX_URL];
    char 	*cp;
    httpDir	*dir;
    httpContent *entry;

    server->response.length = 0;
    strncpy(dirName, server->request.path, HTTP_MAX_URL);

    cp = rindex(dirName, '/');
    if (cp == NULL) {
        printf("Invalid request path '%s'\n",dirName);
        return;
    }
    strncpy(entryName, cp + 1, HTTP_MAX_URL);
    if (cp != dirName)
        *cp = 0;
    else
        *(cp+1) = 0;

    dir = svr_find_dir(server, dirName, HTTP_FALSE);
    if (dir == NULL) {
        svr_send_err404(server);
        svr_write_accesslog(server);
        return;
    }
    entry = svr_find_content(server, dir, entryName);
    if (entry == NULL) {
        svr_send_err404(server);
        svr_write_accesslog(server);
        return;
    }

    if (entry->preload) {
        if ((entry->preload)(server) < 0) {
            svr_write_accesslog(server);
            return;
        }
    }
    switch(entry->type) {
    case SVR_HANDLE_C_FUNCT:
    case SVR_HANDLE_C_WILDCARD:
        (entry->function)(server);
        break;

    case SVR_HANDLE_STATIC:
        svr_send_static(server, entry->data);
        break;

    case SVR_HANDLE_FILE:
        svr_send_file(server, entry->path);
        break;

    case SVR_HANDLE_WILDCARD:
        if (svr_send_direntry(server,entry,entryName) < 0) {
            svr_send_err404(server);
        }
        break;
    }
    svr_write_accesslog(server);
}

void
svr_end_request(httpd * server)
{
    var_exit(&server->variables);
    shutdown(server->clientSock,2);
    close(server->clientSock);
    request_clear(server);
}



void
svr_set_filebase(httpd * server, char * path)
{
    strncpy(server->fileBasePath, path, HTTP_MAX_URL);
}



int
svr_register_file(httpd * server, char * dir, char * name,
                      int indexFlag, int (*preload)(), char * path)
{
    httpDir	*dirPtr;
    httpContent *entry;

    dirPtr = svr_find_dir(server, dir, HTTP_TRUE);
    entry =  malloc(sizeof(httpContent));
    if (entry == NULL)
	return(-1);
    bzero(entry,sizeof(httpContent));
    entry->name = strdup(name);
    entry->type = SVR_HANDLE_FILE;
    entry->indexFlag = indexFlag;
    entry->preload = preload;
    entry->next = dirPtr->entries;
    dirPtr->entries = entry;
#ifdef	__OS2__
    if (*path == '/' || (path[0] && path[1] == ':') )
#else
    if (*path == '/')
#endif
    {
        /* Absolute path */
        entry->path = strdup(path);
    }
    else
    {
        /* Path relative to base path */
        entry->path = malloc(strlen(server->fileBasePath) +
                                strlen(path) + 2);
        snprintf(entry->path, HTTP_MAX_URL, "%s/%s",
                 server->fileBasePath, path);
    }
    return(0);
}



int
svr_register_dir(httpd * server, char * dir, int (*preload)(), char *path)
{
    httpDir	*dirPtr;
    httpContent *entry;

    dirPtr = svr_find_dir(server, dir, HTTP_TRUE);
    entry =  malloc(sizeof(httpContent));
    if (entry == NULL)
        return(-1);
    bzero(entry,sizeof(httpContent));
    entry->name = NULL;
    entry->type = SVR_HANDLE_WILDCARD;
    entry->indexFlag = HTTP_FALSE;
    entry->preload = preload;
    entry->next = dirPtr->entries;
    dirPtr->entries = entry;
#ifdef	__OS2__
    if (*path == '/' || (path[0] && path[1] == ':') )
#else
    if (*path == '/')
#endif
    {
	/* Absolute path */
        entry->path = strdup(path);
    }
    else
    {
        /* Path relative to base path */
        entry->path = malloc(strlen(server->fileBasePath) +
                                strlen(path) + 2);
        snprintf(entry->path, HTTP_MAX_URL, "%s/%s",
                 server->fileBasePath, path);
    }
    return(0);
}




int
svr_register_filehandler(httpd * server, char * dir, char * name,
                         int indexFlag, int (*preload)(), void (*function)())
{
    httpDir	*dirPtr;
    httpContent *entry;

    dirPtr = svr_find_dir(server, dir, HTTP_TRUE);
    entry =  malloc(sizeof(httpContent));
    if (entry == NULL)
        return(-1);
    bzero(entry,sizeof(httpContent));
    entry->name = strdup(name);
    entry->type = SVR_HANDLE_C_FUNCT;
    entry->indexFlag = indexFlag;
    entry->function = function;
    entry->preload = preload;
    entry->next = dirPtr->entries;
    dirPtr->entries = entry;
    return(0);
}


int
svr_register_dirhandler(httpd * server, char * dir,
                            int (*preload)(), void (*function)())
{
    httpDir	*dirPtr;
    httpContent *entry;

    dirPtr = svr_find_dir(server, dir, HTTP_TRUE);
    entry =  malloc(sizeof(httpContent));
    if (entry == NULL)
        return(-1);
    bzero(entry,sizeof(httpContent));
    entry->name = NULL;
    entry->type = SVR_HANDLE_C_WILDCARD;
    entry->indexFlag = HTTP_FALSE;
    entry->function = function;
    entry->preload = preload;
    entry->next = dirPtr->entries;
    dirPtr->entries = entry;
    return(0);
}

int
svr_register_string(httpd * server, char * dir, char * name,
		    int indexFlag, int (*preload)(), char * data)
{
    httpDir	*dirPtr;
    httpContent *entry;

    dirPtr = svr_find_dir(server, dir, HTTP_TRUE);
    entry =  malloc(sizeof(httpContent));
    if (entry == NULL)
        return(-1);
    bzero(entry,sizeof(httpContent));
    entry->name = strdup(name);
    entry->type = SVR_HANDLE_STATIC;
    entry->indexFlag = indexFlag;
    entry->data = data;
    entry->preload = preload;
    entry->next = dirPtr->entries;
    dirPtr->entries = entry;
    return(0);
}

void
svr_send_headers(httpd * server)
{
    svr_send_headers(server);
}

void
svr_set_response(httpd * server, char * msg)
{
    strncpy(server->response.response, msg, HTTP_MAX_URL);
}

void
svr_set_contenttype(httpd * server, char * type)
{
    strncpy(server->response.contentType, type, HTTP_MAX_URL);
}


void
svr_add_header(httpd * server, char * msg)
{
    strcat(server->response.headers, msg);
    if (msg[strlen(msg) - 1] != '\n')
        strcat(server->response.headers, "\n");
}

void
svr_set_cookie(httpd * server, char * name, char * value)
{
    char	buf[HTTP_MAX_URL];

    snprintf(buf,HTTP_MAX_URL, "Set-Cookie: %s=%s; path=/; "
	     "expires=Tue, 01-Jan-2030 00:00:00 GMT",
	     name, value);
    svr_add_header(server, buf);
}

void
svr_expire_cookie(httpd * server, char * name)
{
    char	buf[HTTP_MAX_URL];

    snprintf(buf,HTTP_MAX_URL, "Set-Cookie: %s=expired; path=/; "
	     "expires=Mon, 01-Jan-2001 00:00:00 GMT",
	     name);
    svr_add_header(server, buf);
}

void
svr_use_utf8 (bool val)
{
    server->response.utf8 = val;
}



void
svr_puts(httpd *server, const char *msg)
{
    int len;
    const char * dst;
    char buf[HTTP_MAX_LEN];

    if (server->response.utf8) {
	http_to_utf(buf, msg);
	dst = buf;
    } else
	dst = msg;

    len = strlen(dst);
    server->response.length += len;
    http_send_headers(server, 0, 0);
    write( server->clientSock, dst, len);
}



void
svr_printf(httpd *server, char *fmt, ...)
{
    va_list	args;
    char	tmp[HTTP_MAX_LEN];
    char	buf[HTTP_MAX_LEN*2];
    char*       dst;

    va_start(args, fmt);
    vsnprintf(tmp, HTTP_MAX_LEN, fmt, args);
    va_end(args);

    if (server->response.utf8) {
	http_to_utf(buf, tmp);
	dst = buf;      /* take the converted utf8-string */
    }
    else
        dst = tmp;      /* take the converted utf8-string */

    server->response.length += strlen(dst);
    http_send_headers(server, 0, 0);
    write( server->clientSock, dst, strlen(dst));
}



void
svr_putc(httpd *server, char ch)
{
    if (server->response.utf8) {
        char src[2];
	char buf[4];    /* max bytes for utf8 encoded character */

        src[0] = ch;
        src[1] = '\0';

        http_to_utf(buf, src);
        server->response.length += strlen(buf);
        http_send_headers(server, 0, 0);
        write( server->clientSock, buf, strlen(buf));
    }
    else {
        server->response.length += 1;
        http_send_headers(server, 0, 0);
        write( server->clientSock, &ch, 1);
    }
}



void
svr_set_accesslog(httpd * server, FILE * fp)
{
    server->accessLog = fp;
}

void
svr_set_errorlog(httpd * server, FILE * fp)
{
    server->errorLog = fp;
}

int
svr_check_auth(httpd * server, char * realm)
{
    char    buffer[255];

    if (server->request.authLength == 0) {
        svr_set_response(server, "401 Please Authenticate");
        snprintf(buffer,sizeof(buffer),
                 "WWW-Authenticate: Basic realm=\"%s\"\n", realm);
        svr_add_header(server, buffer);
        svr_puts(server,"\n");
        return(0);
    }
    return(1);
}

void
svr_force_auth(httpd * server, char * realm)
{
    char	buffer[255];

    svr_set_response(server, "401 Please Authenticate");
    snprintf(buffer,sizeof(buffer),
             "WWW-Authenticate: Basic realm=\"%s\"\n", realm);
    svr_add_header(server, buffer);
    svr_puts(server,"\n");
}

httpDir *
svr_find_dir(httpd * server, char * dir, int createFlag)
{
    char	buffer[HTTP_MAX_URL], *curDir;
    httpDir	*curItem, *curChild;

    strncpy(buffer, dir, HTTP_MAX_URL);
    curItem = server->content;
    curDir = strtok(buffer,"/");
    while(curDir)
    {
        curChild = curItem->children;
        while(curChild)
        {
            if (strcmp(curChild->name, curDir) == 0)
                break;
            curChild = curChild->next;
        }
        if (curChild == NULL)
        {
            if (createFlag == HTTP_TRUE)
            {
                curChild = malloc(sizeof(httpDir));
                bzero(curChild, sizeof(httpDir));
                curChild->name = strdup(curDir);
                curChild->next = curItem->children;
                curItem->children = curChild;
            }
            else
            {
                return(NULL);
            }
        }
        curItem = curChild;
        curDir = strtok(NULL,"/");
    }
    return(curItem);
}


int
svr_send_direntry(httpd * server, httpContent * entry, char * entryName)
{
    char		path[HTTP_MAX_URL];

    snprintf(path, HTTP_MAX_URL, "%s/%s", entry->path, entryName);
    svr_send_file(server,path);
    return(0);
}

httpContent *
svr_find_content(httpd * server, httpDir * dir, char * entryName)
{
    httpContent *curEntry;

    curEntry = dir->entries;
    while(curEntry)
    {
        if (curEntry->type == SVR_HANDLE_WILDCARD ||
            curEntry->type == SVR_HANDLE_C_WILDCARD)
            break;
        if (*entryName == 0 && curEntry->indexFlag)
            break;
        if (strcmp(curEntry->name, entryName) == 0)
            break;
        curEntry = curEntry->next;
    }
    if (curEntry)
        server->response.content = curEntry;
    return(curEntry);
}

void
svr_send_static(httpd * server, char * data)
{
    if (http_check_modified(server,server->startTime) == 0) {
        svr_send_err304(server);
    }
    http_send_headers(server, strlen(data), server->startTime);
    svr_puts(server, data);
}



void
svr_send_file(httpd * server, char * path)
{
    char	*suffix;
    struct 	stat sbuf;

    suffix = rindex(path, '.');
    if (suffix != NULL) {
        if (strcasecmp(suffix,".gif") == 0)
            strcpy(server->response.contentType,"image/gif");
        if (strcasecmp(suffix,".jpg") == 0)
            strcpy(server->response.contentType,"image/jpeg");
        if (strcasecmp(suffix,".xbm") == 0)
            strcpy(server->response.contentType,"image/xbm");
        if (strcasecmp(suffix,".png") == 0)
            strcpy(server->response.contentType,"image/png");
    }
    if (stat(path, &sbuf) < 0) {
        svr_send_err404(server);
        return;
    }
    if (http_check_modified(server,sbuf.st_mtime) == 0) {
        svr_send_err304(server);
    }
    else {
        http_send_headers(server, sbuf.st_size, sbuf.st_mtime);
        http_send_file(server, path);
    }
}


void
svr_send_binary(httpd * server, char * data, int len)
{
    if (http_check_modified(server,server->startTime) == 0) {
        svr_send_err304(server);
    }
    server->response.length += len;
    http_send_headers(server, 0, 0);
    //http_send_headers(server, server->startTime, len);
    write(server->clientSock, data, len);
}



void
svr_send_text(httpd * server, char * msg)
{
    server->response.length += strlen(msg);
    write(server->clientSock,msg,strlen(msg));
}



void
svr_send_err304(httpd * server)
{
    svr_set_response(server, "304 Not Modified\n");
    http_send_headers(server,0,0);
}


void
svr_send_err403(httpd * server)
{
    svr_set_response(server, "403 Permission Denied\n");
    http_send_headers(server,0,0);
    svr_send_text(server,
                  "<HTML><HEAD><TITLE>403 Permission Denied</TITLE></HEAD>\n");
    svr_send_text(server,
                  "<BODY><H1>Access to the request URL was denied!</H1>\n");
}


void
svr_send_err404(httpd * server)
{
    char	msg[HTTP_MAX_URL];

    snprintf(msg, HTTP_MAX_URL,
             "File does not exist: %s", server->request.path);
    svr_write_errorlog(server,LEVEL_ERROR, msg);
    svr_set_response(server, "404 Not Found\n");
    http_send_headers(server,0,0);
    svr_send_text(server,
                  "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n");
    svr_send_text(server,
                  "<BODY><H1>The request URL was not found!</H1>\n");
    svr_send_text(server, "</BODY></HTML>\n");
}

void
svr_write_accesslog(httpd * server)
{
    char	dateBuf[30];
    struct 	tm *timePtr;
    time_t	clock;
    int	responseCode;


    if (server->accessLog == NULL)
        return;

    clock = time(NULL);
    timePtr = localtime(&clock);
    strftime(dateBuf, 30, "%d/%b/%Y:%T %Z",  timePtr);
    responseCode = atoi(server->response.response);
    fprintf(server->accessLog, "%s - - [%s] %s \"%s\" %d %d\n",
            server->client_ip, dateBuf, request_get_methodname(server),
            server->request.path, responseCode,
            server->response.length);
    fflush(server->accessLog);
}

void
svr_write_errorlog(httpd * server, char * level, char * message)
{
    char	dateBuf[30];
    struct 	tm *timePtr;
    time_t	clock;

    if (server->errorLog == NULL)
	return;

    clock = time(NULL);
    timePtr = localtime(&clock);
    strftime(dateBuf, 30, "%a %b %d %T %Y",  timePtr);
    if (*server->client_ip != 0) {
        fprintf(server->errorLog, "[%s] [%s] [client %s] %s\n",
                dateBuf, level, server->client_ip, message);
    }
    else {
        fprintf(server->errorLog, "[%s] [%s] %s\n",
                dateBuf, level, message);
    }
    fflush(server->accessLog);
}




#if SETUID
    /* If we're root, try to become someone else. */
    if ( getuid() == 0 ) {
	/* Set aux groups to null. */
	if ( setgroups( 0, (const gid_t*) 0 ) < 0 ) {
	    syslog( LOG_CRIT, "setgroups - %m" );
	    exit( 1 );
	    }
	/* Set primary group. */
	if ( setgid( gid ) < 0 ) {
	    syslog( LOG_CRIT, "setgid - %m" );
	    exit( 1 );
	    }
	/* Try setting aux groups correctly - not critical if this fails. */
	if ( initgroups( user, gid ) < 0 )
	    syslog( LOG_WARNING, "initgroups - %m" );

#ifdef HAVE_SETLOGIN
	/* Set login name. */
        (void) setlogin( user );
#endif /* HAVE_SETLOGIN */
	/* Set uid. */
	if ( setuid( uid ) < 0 ) {
	    syslog( LOG_CRIT, "setuid - %m" );
	    exit( 1 );
	}
	/* Check for unnecessary security exposure. */
	if ( ! do_chroot )
	    syslog( LOG_CRIT,
		   "started as root without requesting chroot(), warning only" );
    }

#endif


#if CHROOT
    /* Chroot if requested. */
    if ( do_chroot ) {
	if ( chroot( cwd ) < 0 ) {
	    syslog( LOG_CRIT, "chroot - %m" );
	    perror( "chroot" );
	    exit( 1 );
	}
	(void) strcpy( cwd, "/" );
	/* Always chdir to / after a chroot. */
	if ( chdir( cwd ) < 0 ) {
	    syslog( LOG_CRIT, "chroot chdir - %m" );
	    perror( "chroot chdir" );
	    exit( 1 );
	}
    }
#endif
