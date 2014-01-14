/*
 * page.h - routines to do all the things with our pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef MISC_H
#define MISC_H



#include "types.h"



/* Misc functions */
int             get_time();
bool		is_image(char * image);
bool            is_wikiword(const char* tptr);
void 		convert_to_wikiword(char * string);
char*		make_spaced_title(const char* title);
void            xml_putc(char ch);
void            xml_puts(char * str);

#endif
