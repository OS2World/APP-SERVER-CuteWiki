/*
 * request.c - webserver's request handling
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
#include <time.h>

#include <unistd.h> 

#include "config.h"
#include "svr.h"
#include "misc.h"
#include "http.h"



#define RET_ILSEQ     0
#define RET_TOOSMALL  0
#define RET_TOOFEW    0



/*
 * request_to_iso - convert UTF-8-String to Latin1
 */
static int
request_to_iso (char *str)
{
    unsigned char *s = (unsigned char*)str;
    unsigned char *dest = (unsigned char*)str;

    dest = (unsigned char*)s;
    /* in UTF-8 there are no 0 bytes other then 0! */
    while (*s) {
        wchar_t wc;

        if (s[0] < 0x80) {
            wc = *s++;
        }
        else if (s[0] < 0xc2) {
            return RET_ILSEQ;
        }
        else if (s[0] < 0xe0) {
            if (!s[1])
                return RET_TOOFEW;
            if (!((s[1] ^ 0x80) < 0x40))
                return RET_ILSEQ;
            wc  = (wchar_t) (*s++ & 0x1f) << 6;
            wc |= (wchar_t) (*s++ ^ 0x80);
        }
        else if (s[0] < 0xf0) {
            if (!s[1] || !s[2])
                return RET_TOOFEW;
            if (!((s[1] ^ 0x80) < 0x40
                  && (s[2] ^ 0x80) < 0x40
                  && (s[0] >= 0xe1 || s[1] >= 0xa0)))
                return RET_ILSEQ;
            wc  = (wchar_t) (*s++ & 0x0f) << 12;
            wc |= (wchar_t) (*s++ ^ 0x80) << 6;
            wc |= (wchar_t) (*s++ ^ 0x80);
        }
        else if (s[0] < 0xf8 && sizeof(wchar_t)*8 >= 32) {
            if (!s[1] || !s[2] || !s[3])
                return RET_TOOFEW;
            if (!((s[1] ^ 0x80) < 0x40
                  && (s[2] ^ 0x80) < 0x40
                  && (s[3] ^ 0x80) < 0x40
                  && (s[0] >= 0xf1 || s[1] >= 0x90)))
                return RET_ILSEQ;
            wc  = (wchar_t) (*s++ & 0x07) << 18;
            wc |= (wchar_t) (*s++ ^ 0x80) << 12;
            wc |= (wchar_t) (*s++ ^ 0x80) << 6;
            wc |= (wchar_t) (*s++ ^ 0x80);
        }
        else if (s[0] < 0xfc && sizeof(wchar_t)*8 >= 32) {
            if (!s[1] || !s[2] || !s[3] || !s[4])
                return RET_TOOFEW;
            if (!((s[1] ^ 0x80) < 0x40
                  && (s[2] ^ 0x80) < 0x40
                  && (s[3] ^ 0x80) < 0x40
                  && (s[4] ^ 0x80) < 0x40
                  && (s[0] >= 0xf9 || s[1] >= 0x88)))
                return RET_ILSEQ;
            wc  = (wchar_t) (*s++ & 0x03) << 24;
            wc |= (wchar_t) (*s++ ^ 0x80) << 18;
            wc |= (wchar_t) (*s++ ^ 0x80) << 12;
            wc |= (wchar_t) (*s++ ^ 0x80) << 6;
            wc |= (wchar_t) (*s++ ^ 0x80);
        }
        else if (s[0] < 0xfe && sizeof(wchar_t)*8 >= 32) {
            if (!s[1] || !s[2] || !s[3] || !s[4] || !s[5])
                return RET_TOOFEW;
            if (!((s[1] ^ 0x80) < 0x40
                  && (s[2] ^ 0x80) < 0x40
                  && (s[3] ^ 0x80) < 0x40
                  && (s[4] ^ 0x80) < 0x40
                  && (s[5] ^ 0x80) < 0x40
                  && (s[0] >= 0xfd || s[1] >= 0x84)))
                return RET_ILSEQ;
            wc  = (wchar_t) (*s++ & 0x01) << 30;
            wc |= (wchar_t) (*s++ ^ 0x80) << 24;
            wc |= (wchar_t) (*s++ ^ 0x80) << 18;
            wc |= (wchar_t) (*s++ ^ 0x80) << 12;
            wc |= (wchar_t) (*s++ ^ 0x80) << 6;
            wc |= (wchar_t) (*s++ ^ 0x80);
        }
        else {
            return RET_ILSEQ;
        }

        /* Now convert to iso-8859-1 */
        if (wc < 0x100)
            *dest++ = wc;
        else
            *dest++ = '_';      /* a character not in ISO-8859-1 */
    }
    *dest = '\0';  		/* force the ending zero */

    return 1;
}


static char
request_from_hex (char c)
{
    return  c >= '0' && c <= '9' ?  c - '0'
        : c >= 'A' && c <= 'F'? c - 'A' + 10
        : c - 'a' + 10;     /* accept small letters just in case */
}



static char *
request_unescape(char *str)
{
    char * p = str;
    char * q = str;
    static char blank[] = "";

    if (!str)
        return(blank);
    while (*p) {
        if (*p == '%') {
            p++;
            if (*p) *q = request_from_hex(*p++) * 16;
            if (*p) *q = (*q + request_from_hex(*p++));
            q++;
        } else {
            if (*p == '+') {
                *q++ = ' ';
                p++;
            } else {
                *q++ = *p++;
              }
        }
    }
    *q = 0;
    request_to_iso(str);

    return str;
} 



char *
request_get_methodname(httpd * server)
{
    static char	buf[255];

    switch(server->request.method) {
    case HTTP_GET:
        return("GET");
    case HTTP_POST:
        return("POST");
    default:
        snprintf(buf,255,"Invalid method '%d'", server->request.method);
        return(buf);
    }
}

void
request_clear(httpd * server)
{
    bzero(&server->request, sizeof(server->request));
}

int
request_get_method(httpd * server)
{
    return server->request.method;
}

char *
request_get_uri(httpd * server)
{
    return request_unescape(server->request.path);
}

int
request_get_length(httpd * server)
{
    return server->request.contentLength;
}

char *
request_get_type(httpd * server)
{
    return server->request.contentType;
}

/*
 * Returns the start time of the request
 */
int
request_get_start(httpd * server)
{
    return server->request.starttime;
}


static void
request_store_data(httpd *server, char *query)
{
    char *cp, *cp2;
    char var[50];
    char *val, *tmp;

    if (!query)
        return;

    cp = query;
    cp2 = var;
    bzero(var,sizeof(var));
    val = NULL;
    while (*cp) {
        if (*cp == '=') {
            cp++;
            *cp2 = 0;
            val = cp;
            continue;
        }
        if (*cp == '&') {
            *cp = 0;
            tmp = request_unescape(val);
            var_set(&server->variables, var, tmp);
            cp++;
            cp2 = var;
            val = NULL;
            continue;
        }
        if (val) {
            cp++;
        }
        else {
            *cp2 = *cp++;
            if (*cp2 == '.') {
                strcpy(cp2,"_dot_");
                cp2 += 5;
            }
            else {
                cp2++;
            }
        }
    }
    *cp = 0;
    tmp = request_unescape(val);
    var_set(&server->variables, var, tmp);
}



static int
request_decode (char *bufcoded, char *bufplain, int outbufsize)
{
    static char six2pr[64] = {
        'A','B','C','D','E','F','G','H','I','J','K','L','M',
        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        'a','b','c','d','e','f','g','h','i','j','k','l','m',
        'n','o','p','q','r','s','t','u','v','w','x','y','z',
        '0','1','2','3','4','5','6','7','8','9','+','/'
    };

    static unsigned char pr2six[256];

    /* single character decode */
#define DEC(c) pr2six[(int)c]
#define _DECODE_MAXVAL 63

    static int first = 1;

    int nbytesdecoded, j;
    char *bufin = bufcoded;
    unsigned char *bufout = (unsigned char*)bufplain;
    int nprbytes;

    /*
     ** If this is the first call, initialize the mapping table.
     ** This code should work even on non-ASCII machines.
     */
    if (first) {
        first = 0;
        for (j=0; j<256; j++) pr2six[j] = _DECODE_MAXVAL+1;
        for (j=0; j<64; j++) pr2six[(int)six2pr[j]] = (unsigned char)j;
    }

    /* Strip leading whitespace. */

    while (*bufcoded==' ' || *bufcoded == '\t')
        bufcoded++;

    /*
     ** Figure out how many characters are in the input buffer.
     ** If this would decode into more bytes than would fit into
     ** the output buffer, adjust the number of input bytes downwards.
     */
    bufin = bufcoded;
    while (pr2six[(int)*(bufin++)] <= _DECODE_MAXVAL);
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;
    if (nbytesdecoded > outbufsize) {
        nprbytes = (outbufsize*4)/3;
    }
    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++)=(unsigned char)(DEC(*bufin)<<2|DEC(bufin[1])>>4);
        *(bufout++)=(unsigned char)(DEC(bufin[1])<<4|DEC(bufin[2])>>2);
        *(bufout++)=(unsigned char)(DEC(bufin[2])<<6|DEC(bufin[3]));
        bufin += 4;
        nprbytes -= 4;
    }
    if (nprbytes & 03) {
        if (pr2six[(int)bufin[-2]] > _DECODE_MAXVAL) {
            nbytesdecoded -= 2;
        }
        else {
            nbytesdecoded -= 1;
        }
    }
    bufplain[nbytesdecoded] = 0;
    return(nbytesdecoded);
}



static void
request_sanitise_url(char *url)
{
    char	*from,
    *to,
    *last;

    /* Remove multiple slashes */
    from = to = url;
    while (*from) {
        if (*from == '/' && *(from+1) == '/') {
            from++;
            continue;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;


    /* Get rid of ./ sequences */
    from = to = url;
    while (*from) {
        if (*from == '/' && *(from+1) == '.' && *(from+2)=='/') {
            from += 2;
            continue;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;


    /*
     ** Catch use of /../ sequences and remove them.  Must track the
     ** path structure and remove the previous path element.
     */
    from = to = last = url;
    while (*from) {
        if (*from == '/' && *(from+1) == '.' &&
            *(from+2)=='.' && *(from+3)=='/') {
            to = last;
            from += 3;
            continue;
        }
        if (*from == '/') {
            last = to;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;
}



int
request_read(httpd * server, char * req)
{
    int	count;
    int in_headers;
    char *cp, *cp2;

    server->request.starttime = get_time();     /* for time measurement */

    /* Read the request */
    count = 0;
    in_headers = 1;
    while(http_read_line(server, req, HTTP_MAX_LEN) > 0) {

        /* Special case for the first line.  */
	count++;
        if (count == 1) {
            /* First line.  Scan the request info */
            cp = cp2 = req;
            while(isalpha(*cp2))
                cp2++;
            *cp2 = 0;
            if (strcasecmp(cp,"GET") == 0)
                server->request.method = HTTP_GET;
            if (strcasecmp(cp,"POST") == 0)
                server->request.method = HTTP_POST;
	    if (server->request.method == 0) {
                /* method unknown */
		return(-1);
            }
            cp = cp2+1;
            while(*cp == ' ')
                cp++;
            cp2 = cp;
            while(*cp2 != ' ' && *cp2 != 0)
                cp2++;
            *cp2 = 0;
            strncpy(server->request.path,cp,HTTP_MAX_URL);
	    request_sanitise_url(server->request.path);
            continue;
        }

        /* Process the headers */
        if (in_headers) {
            if (*req == 0) {
                /* End of headers.  Continue if there's data to read */
                if (server->request.contentLength == 0)
                    break;
                in_headers = 0;
                break;
            }
            if (strncasecmp(req,"Cookie: ",7) == 0) {
                char	*var,
                *val,
                *end;

                var = index(req,':');
                while(var) {
                    var++;
                    val = index(var, '=');
                    *val = 0;
                    val++;
                    end = index(val,';');
		    if(end)
			*end = 0;
                    var_set(&server->variables, var, val);
                    var = end;
                }
            }
            if (strncasecmp(req,"Authorization: ",15) == 0) {
                cp = index(req,':') + 2;
                if (strncmp(cp,"Basic ", 6) != 0) {
                    /* Unknown auth method */
                }
                else {
                    char 	authBuf[100];

                    cp = index(cp,' ') + 1;
                    request_decode(cp, authBuf, 100);
                    server->request.authLength = strlen(authBuf);
                    cp = index(authBuf,':');
                    if (cp) {
                        *cp = 0;
                        strncpy(server->request.authPassword,
                                cp+1, HTTP_MAX_AUTH);
                    }
                    strncpy(server->request.authUser,
                            authBuf, HTTP_MAX_AUTH);
                }
            }
            if (strncasecmp(req,"Referer: ",9) == 0) {
                cp = index(req,':') + 2;
                if(cp) {
                    strncpy(server->request.referer,cp,
			    HTTP_MAX_URL);
                }
            }
            if (strncasecmp(req,"If-Modified-Since: ",19) == 0) {
                cp = index(req,':') + 2;
                if(cp) {
                    strncpy(server->request.ifModified,cp,
                            HTTP_MAX_URL);
                    cp = index(server->request.ifModified,
                               ';');
                    if (cp)
                        *cp = 0;
                }
            }
            if (strncasecmp(req,"Content-Type: ",14) == 0) {
                cp = index(req,':') + 2;
                if(cp) {
                    strncpy(server->request.contentType,cp,
                            HTTP_MAX_URL);
                }
            }
            if (strncasecmp(req,"Content-Length: ",16) == 0) {
                cp = index(req,':') + 2;
                if(cp)
                    server->request.contentLength = atoi(cp);
            }
            continue;
        }
    }

    /* Process POST data */
    if (server->request.contentLength > 0) {
        bzero(req, HTTP_MAX_LEN);
        http_read_buf(server, req, server->request.contentLength);
        request_store_data(server, req);
    }

    /* Process any URL data */
    cp = index(server->request.path,'?');
    if (cp != NULL) {
        *cp = 0;
        cp++;
        request_store_data(server, cp);
    }
    return(0);
}
