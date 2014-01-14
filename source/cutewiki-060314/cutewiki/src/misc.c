/*
 * meta.c - Write out the meta pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "cutewiki.h"



int
get_time ()
{
    struct timeval tv;

    gettimeofday (&tv, NULL);
    return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}



/*
 * Get full path of image file
 */
static bool
get_imagefilename(char * image, char * fn)
{
    if (image) {
        snprintf(fn, MAX_PATH, "%s/%s.png", wiki_get_imagedir(), image);
        return true;
    }

    return false;
}



bool
is_image(char * image)
{
    char	fn[MAX_PATH];
    struct stat fstat;

    /* load page's text */
    get_imagefilename(image, fn);

    /* see, if file exists */
    if (stat(fn, &fstat) == 0)
        return true;

    return false;
}



#if 0
/*
 * is_wikiword - Check, if a string is a valid WikiWord
 *
 * This new implementation makes use of the current locale setting.
 */
bool
is_wikiword (const char* word)
{
    bool wikiword = false;

    if (word == NULL)
        return false;

    /* Must start with an alpha character */
    if (!isalpha((unsigned char)*word))
        return false;

    /* See, if in den middle of the word there is a number or a Capitel */
    while (*++word) {
        if (isupper((unsigned char)*word))
            wikiword = true;;
        if (isdigit((unsigned char)*word))
            wikiword = true;
        if (*word == '_')
            wikiword = true;
    }

    return wikiword;
}
#endif


/*
 * is_wikiword - Check, if a string is a valid WikiWord
 *
 * This new imlementation makes use of the current locale setting.
 */
bool
is_wikiword (const char* word)
{
    if (word == NULL)
        return false;

    if (strlen(word) < 3)
        return false;

    /* Must start with an uppercase character */
    if (!isupper((unsigned char)*word))
        return false;
    word++;
    /* next must be lowercase */
    if (!islower((unsigned char)*word))
        return false;
    word++;

    /* skip all small characters */
    while (islower((unsigned char)*word))
	word++;

    /* Now capitals or numerals */
    if (!isupper((unsigned char)*word) && !isdigit((unsigned char)*word))
        return false;

    /* And no invalid rubbish afterwards */
    while (*++word) {
	if (isupper((unsigned char)*word))
            continue;
        if (islower((unsigned char)*word))
            continue;
        if (isdigit((unsigned char)*word))
            continue;
        return false;
    }

    return true;
}

char *
my_strncpy(char *dest, const char *src, int n)
{
    int count = 0;

    while(*src && count < n) {
        *dest++ = *src++;
        count++;
    }
    *dest = '\0';

    return dest;
}

#if 0
/*
 * Make a copy of the string, but as WikiWord
 */
void
convert_to_wikiword(char * string)
{
    char*	src;
    char*	dst;
    bool        newword;

    /* FIXME: Not Unicode ready! */
    src = string;
    dst = string;
    newword = true;
    while(*src) {
        if (isspace((unsigned char)*src)) {
            newword = true;
            src++;
            continue;
        }

        if (newword)
            *dst++ = toupper((unsigned char)*src++);
        else
            *dst++ = *src++;

        newword = false;
    }
    *dst = '\0';
}
#endif


char*
make_spaced_title(const char* title)
{
    const char*	tptr;
    int		caps;
    char*	obuf;
    char*	optr;

    tptr = title;
    caps = 0;
    while (*tptr) {
        if (isupper((unsigned char)*tptr))
            caps++;
        tptr++;
    }
    caps--;	/* ignore first */

    obuf = malloc(sizeof(char) * (strlen(title) + caps + 1));
    optr = obuf;
    tptr = title;
    *optr++ = *tptr++;
    while (*tptr) {
        if (isupper((unsigned char)*tptr))
            *optr++ = ' ';	/* but I think I prefer no space ;) */
        *optr++ = *tptr++;
    }
    *optr++ = 0;

    return obuf;
}

/*
 * xml_putc - outputs a character to the server
 */
void
xml_putc(char ch)
{
    switch (ch) {
    case '&':
        svr_puts(server, "&amp;");
        break;
    case '\"':
        svr_puts(server, "&quot;");
        break;
    case '<':
        svr_puts(server, "&lt;");
        break;
    case '>':
        svr_puts(server, "&gt;");
        break;
    case '%':
        svr_puts(server, "&#x25;");
        break;
    case 0x0d:
        /* skip dos like line ending */
        break;
    default:
        svr_putc(server, ch);
    }
}



/*
 * wui_Puts - outputs a string to the server
 */
void
xml_puts(char * str)
{
    if (str != NULL) {
        char ch;

        while ((ch = *str++)) {
            xml_putc(ch);
        }
    }
}



