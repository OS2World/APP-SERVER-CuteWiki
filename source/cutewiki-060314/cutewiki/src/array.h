/*
 * array.h - dynamicly growing array
 *
 * Copyright 2003, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef ARRAY_H
#define ARRAY_H


struct array
{
    void ** data;
    size_t length;
    size_t max;
};


struct array *	array_new();
void	array_del(struct array * array);
void	array_add_string(struct array * array, char * string, size_t maxlen);
bool    array_has_string(struct array * array, char * string, size_t maxlen);

#endif
