/*
 * array.c - dynamicly growing array routines
 *
 * Copyright 2005, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "array.h"



#define INITIAL_ARRAY_LENGHT 32

/*
 * array_new - create a new array of void pointers
 */
struct array *
array_new()
{
    struct array * array;

    /* create array struct to hold information */
    array = (struct array *)calloc(1, sizeof(struct array));

    /* create the array of pointers itself */
    array->data = (void **)calloc(INITIAL_ARRAY_LENGHT, sizeof(void*));
    array->length = 0;
    array->max = INITIAL_ARRAY_LENGHT;

    return array;
}

/*
 * array_del - delete the whole array
 *
 * Beware: just use this, if alloced data don't need to be freed!
 */
void
array_del(struct array * array)
{
    free(array->data);
    free(array);
}

/*
 * array_grow - make the array bigger
 *
 * This may be called, if the original array can not hold enough data
 */
static struct array *
array_grow(struct array * array)
{
    size_t i;
    void ** data;
    size_t max;

    /* copy contents to a new and bigger array */
    max = array->max * 2;

    data = (void **)calloc(max, sizeof(void*));
    for (i = 0; i < array->length; i++) {
	data[i] = array->data[i];
    }
    free(array->data);

    /* update information in array struct */
    array->data = data;
    array->max = max;

    return array;
}

/*
 * array_add_item - insert a new array item
 */
void
array_add_string(struct array * array, char * string, size_t maxlen)
{
    size_t pos;

    if (string == NULL || array == NULL)
        return;

    /* is the string already in the array? */
    for (pos = 0; pos < array->length; pos++) {
	if (!strncmp((char *)array->data[pos], string, maxlen))
	    break;
    }

    /* if string was not in array, add it */
    if (pos == array->length) {
	if (array->length == array->max)
	    array_grow(array);
        array->data[pos] = string;
    }
}


/*
 * array_has_string - test, if a string is in the array
 */
bool
array_has_string(struct array * array, char * string, size_t maxlen)
{
    size_t pos;

    if (string == NULL || array == NULL)
        return false;

    /* is the string in the array? */
    for (pos = 0; pos < array->length; pos++) {
	if (!strncmp((char *)array->data[pos], string, maxlen))
	    return true;
    }

    return false;
}

