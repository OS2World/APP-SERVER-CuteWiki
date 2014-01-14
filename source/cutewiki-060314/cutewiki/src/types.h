/*
 * types.h - All the commonly used things
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef TYPES_H
#define TYPES_H

#include <time.h>



typedef enum { false, true } bool;

enum ViewModes
{
    MODE_NORMAL  = 1,
    MODE_REVERSE,
    MODE_EDIT,
    MODE_HTML,
    MODE_ASCII,
    MODE_RSS,
    MODE_PRINT,
    MODE_RTF
};



typedef int (*CompFunc)(const void* self, const void* other);
typedef void (*SetFunc) (void* value, void* data);
typedef bool (*ExecFunc) (void* value, void* data);
typedef bool (*CheckFunc) (void* value);
typedef void* (*GetFunc) (void* value, const void* data);



#endif
