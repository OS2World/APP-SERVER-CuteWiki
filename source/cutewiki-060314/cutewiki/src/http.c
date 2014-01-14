/*
 * http.c - Implementation of the http protocol
 *
 * Copyright 2002 Hughes Technologies Pty Ltd.  All rights reserved.
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/file.h>

#include "types.h"
#include "config.h"
#include "svr.h"
#include "http.h"
#include "var.h"



/*
 * http_to_utf - convert Latin1 to UTF-8-String
 */
void
http_to_utf (char * dst, const char * src)
{
    const char *sptr = src;
    char *dptr = dst;

    while (*sptr) {
        /* if 7 bit ASCII character, just copy */
        if ((unsigned char)*sptr < 0x80) {
            *dptr = *sptr;
            dptr++;
        }
        /* really just do it for the 8 bits of iso-8859-1! */
        else {
            *dptr = (0xc0 | ((unsigned char)*sptr >> 6) );
            dptr++;
            *dptr = (0x80 | (*sptr & 0x3f) );
            dptr++;
        }
        sptr++;
    }
    *dptr = '\0';  // end the new string
}



#if 0
char *
http_to_utf (const unsigned char *s)
{
    char *buf;
    static int loop = 0;
    static char dest[1024*1024][5];

    if (s == NULL) {
        return NULL;
    }

    if (loop >= 4)
        loop = 0;

    buf = dest[loop];
    while (*s) {
        /* if 7 bit ASCII character, just copy */
        if (*s < 0x80) {
            *buf = *s;
            buf++;
        }
        /* really just do it for the 8 bits of iso-8859-1! */
        else {
            *buf = (0xc0 | (*s >> 6) );
            buf++;
            *buf = (0x80 | (*s & 0x3f) );
            buf++;
        }
        s++;
    }
    *buf = '\0';  // because we need the ending zero

    /* return newly created UTF-8-String */
    return dest[loop++];
}
#endif



int
http_read_char(httpd *server, char *cp)
{
    if (server->readBufRemain == 0) {
        bzero(server->readBuf, HTTP_READ_BUF_LEN + 1);
        server->readBufRemain =
	    read(server->clientSock, server->readBuf, HTTP_READ_BUF_LEN);
        if (server->readBufRemain < 1)
            return 0;
        server->readBuf[server->readBufRemain] = 0;
        server->readBufPtr = server->readBuf;
    }
    *cp = *server->readBufPtr++;
    server->readBufRemain--;
    return(1);
}



int
http_read_line(httpd *server, char *buf, int len)
{
    char	curChar,
    *dst;
    int	count;

    count = 0;
    dst = buf;
    while (count < len) {
        if (http_read_char(server, &curChar) < 1)
            return 0;
        if (curChar == '\n') {
            *dst = 0;
            return(1);
        }
        if (curChar == '\r') {
            continue;
        }
        else {
            *dst++ = curChar;
            count++;
        }
    }
    *dst = 0;
    return(1);
}



int
http_read_buf(httpd *server, char *buf, int len)
{
    char	curChar,
    *dst;
    int	count;


    count = 0;
    dst = buf;
    while (count < len) {
        if (http_read_char(server, &curChar) < 1)
            return 0;
        *dst++ = curChar;
        count++;
    }
    return(1);
}



static void
http_get_timestr(httpd *server, char *ptr, time_t clock)
{
    struct 	tm *timePtr;

    if (clock == 0)
        clock = time(NULL);
    timePtr = gmtime((time_t*)&clock);
    strftime(ptr, HTTP_TIME_STRING_LEN,"%a, %d %b %Y %T GMT",timePtr);
}



void
http_send_headers(httpd *server, int contentLength, int modTime)
{
    char	tmpBuf[80],
    timeBuf[HTTP_TIME_STRING_LEN];

    if (server->response.headersSent)
        return;

    server->response.headersSent = true;
    write(server->clientSock, "HTTP/1.0 ", 9);
    write(server->clientSock, server->response.response,
                   strlen(server->response.response));
    write(server->clientSock, server->response.headers,
                   strlen(server->response.headers));

    http_get_timestr(server, timeBuf, 0);
    write(server->clientSock,"Date: ", 6);
    write(server->clientSock, timeBuf, strlen(timeBuf));
    write(server->clientSock, "\n", 1);

    write(server->clientSock, "Connection: close\n", 18);
    write(server->clientSock, "Content-Type: ", 14);
    write(server->clientSock, server->response.contentType,
                   strlen(server->response.contentType));
    write(server->clientSock, "\n", 1);

    if (contentLength > 0) {
        write(server->clientSock, "Content-Length: ", 16);
        snprintf(tmpBuf, sizeof(tmpBuf), "%d", contentLength);
        write(server->clientSock, tmpBuf, strlen(tmpBuf));
        write(server->clientSock, "\n", 1);

        http_get_timestr(server, timeBuf, modTime);
        write(server->clientSock, "Last-Modified: ", 15);
        write(server->clientSock, timeBuf, strlen(timeBuf));
        write(server->clientSock, "\n", 1);
    }
    write(server->clientSock, "\n", 1);
}



int
http_check_modified(httpd *server, int modTime)
{
    char 	timeBuf[HTTP_TIME_STRING_LEN];

    http_get_timestr(server, timeBuf, modTime);
    if (strcmp(timeBuf, server->request.ifModified) == 0)
        return 0;
    return(1);
}



static unsigned char isAcceptable[96] =

/* Overencodes */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2

/*      Bit 0           xalpha          -- see HTFile.h
**      Bit 1           xpalpha         -- as xalpha but with plus.
**      Bit 2 ...       path            -- as xpalpha but with /
*/
    /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    {    7,0,0,0,0,0,0,0,0,0,7,0,0,7,7,7,       /* 2x   !"#$%&'()*+,-./ */
         7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,       /* 3x  0123456789:;<=>?  */
         7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 4x  @ABCDEFGHIJKLMNO */
         7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,       /* 5X  PQRSTUVWXYZ[\]^_ */
         0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 6x  `abcdefghijklmno */
         7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0 };     /* 7X  pqrstuvwxyz{\}~ DEL */
 
#define ACCEPTABLE(a)   ( a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

static char *hex = "0123456789ABCDEF";


char *
http_escape(char *str)
{
    unsigned char mask = URL_XPALPHAS;
    char * p;
    char * q;
    char * result;
    int unacceptable = 0;

    for (p=str; *p; p++)
        if (!ACCEPTABLE((unsigned char)*p))
                unacceptable +=2;
    result = (char *) malloc(p-str + unacceptable + 1);
    bzero(result,(p-str + unacceptable + 1));

    if (result == NULL) {
	return(NULL);
    }

    for (q=result, p=str; *p; p++) {
        unsigned char a = *p;
        if (!ACCEPTABLE(a)) {
            *q++ = '%';  /* Means hex commming */
            *q++ = hex[a >> 4];
            *q++ = hex[a & 15];
        }
        else *q++ = *p;
    }
    *q++ = 0;                   /* Terminate */

    return result;
}



void
http_send_file(httpd *server, char *path)
{
    int	fd,
    len;
    char	buf[HTTP_MAX_LEN];

    fd = open(path,O_RDONLY);
    if (fd < 0)
        return;
    len = read(fd, buf, HTTP_MAX_LEN);
    while (len > 0) {
        server->response.length += len;
        write(server->clientSock, buf, len);
        len = read(fd, buf, HTTP_MAX_LEN);
    }
    close(fd);
}



