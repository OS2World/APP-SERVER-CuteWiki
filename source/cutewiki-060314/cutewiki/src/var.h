/*
 * var.h - handling of key / value pairs in dynamic variables
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef VAR_H
#define VAR_H



#include "types.h"

typedef struct Var_ Var;
struct Var_ {
    char	*name;
    char 	*value;
    Var 	*next;
};



void 	var_set(Var**, char*, char*);
void    var_del(Var** start, char* name);
bool    var_get_bool(Var* start , char * name);
int	var_get_int(Var* start, char * name);
char*	var_get_val(Var* start, char * name);
void 	var_exit(Var**);



#endif
