/*
 * parse.h - The parser for cutewiki's ASCII pages
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef PARSER_H
#define PARSER_H



#include "page.h"



#define MAX_NUMFOOT     256

/*
 * option for different outputs
 */
typedef struct Output Output;
struct Output
{
    void (*Putc)(char ch);
    void (*Puts)(char * str);
    void (*page_header)(Page * page, int mode);
    void (*page_footer)(Page * page, int mode);
    void (*ParaBegin)();
    void (*ParaEnd)();
    void (*PreBegin)();
    void (*PreEnd)();
    void (*BlockquoteBegin)();
    void (*BlockquoteEnd)();
    void (*RulerBegin)();
    void (*RulerEnd)();
    void (*ListBegin)();
    void (*ListEnd)();
    void (*NumListBegin)();
    void (*NumListEnd)();
    void (*ListItemBegin)();
    void (*ListItemEnd)();
    void (*LineBegin)();
    void (*LineEnd)();
    void (*HeadingBegin)(int level);
    void (*HeadingEnd)(int level);
    void (*Footnote)(char * num);
    void (*BoldBegin)();
    void (*BoldEnd)();
    void (*ItalicBegin)();
    void (*ItalicEnd)();
    void (*InternalLink)(char * name, char * title, Pagetype type);
    void (*BrokenLink)(char * name);
    void (*TableBegin)(int cells);
    void (*TableEnd)();
    void (*TableHeadBegin)();
    void (*TableHeadEnd)();
    void (*TableRowBegin)();
    void (*TableRowEnd)();
    void (*TableCellBegin)();
    void (*TableCellEnd)();
    void (*TableNumberBegin)();
    void (*TableNumberEnd)();
    void (*image)();
    void (*url)(char * url);
    void (*image_url)(char * url);
    void (*external_link)(char * url, char * text);
};



/* Pluggable output options */

extern Output* out;
extern Output htm;
extern Output prt;
extern Output txt;
extern Output rss;
extern Output rtf;



void		out_print_page(Page * page, int mode);
void 		out_write_page(char * pname, int mode);
void 		out_write_error(char *, char *, char *);

char* 		get_alnum(char** string);



#endif
