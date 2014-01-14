/*
 * create.h - create very important pages on demand
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "types.h"



#ifndef CREATE_H
#define CREATE_H


bool    create_wikiadmin();
bool    create_special_page(char * name);
bool    create_is_special(char * name);



#endif
