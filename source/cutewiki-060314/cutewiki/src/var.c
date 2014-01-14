/*
 * var.c - handling of key / value pairs in dynamic variables
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

#include "var.h"


/*
 * var_set - save a variable and it's value
 *
 * There can be more than one setting of a specific variable in a
 * HTTP request. So we need to save them all.
 */
void
var_set(Var** head, char* name, char* value)
{
    Var * var;

    if (name == NULL || value == NULL)
	return;

    while(*name == ' ' || *name == '\t')
	name++;

    /* create the var */
    var = calloc(1, sizeof(Var));
    var->name = strdup(name);
    var->value = strdup(value);

    /* make it the new head of the list */
    var->next = *head;
    *head = var;
}



/*
 * var_get - get a saved variable
 *
 * Although there can be more than one instance of a named variable
 * we are just able to get back the value of the first accurance.
 */
static Var *
var_get(Var* var, char* name)
{
    if (name == NULL)
	return NULL;

    while(var != NULL && var->name != NULL) {
        if (strcmp(var->name, name) == 0)
            return(var);
        var = var->next;
    }

    return NULL;
}



/*
 * var_get_bool - get a boolean from a form
 */
bool
var_get_bool(Var* head, char * name) {
    Var* var;

    var = var_get(head, name);
    if (var != NULL && var->value != NULL) {
	if (strcmp(var->value, "yes") == 0)
	    return true;
    }

    return false;
}



/*
 * var_get_int - get's a value from the form as integer
 */
int
var_get_int(Var* head, char * name)
{
    Var* var;

    var = var_get(head, name);
    if (var != NULL && var->value != NULL)
	return atoi(var->value);

    return 0;
}



/*
 * var_get_val - get's a validated value from the form's data
 *
 * If a variables's value is empty, return NULL
 */
char*
var_get_val(Var* head, char * name)
{
    Var* var;

    var = var_get(head, name);
    if (var != NULL && var->value != NULL) {
	if (strlen(var->value) > 0)
	    return var->value;
    }

    return NULL;
}



/*
 * var_del - delete a variable and it's value
 *
 * because a variable can appear any times, delete then all
 */
void
var_del(Var** head, char* name)
{
    Var *var, *last;

    if (name == NULL )
	return;

    while(*name == ' ' || *name == '\t')
	name++;

    last = NULL;
    var = *head;
    while(var != NULL && var->name != NULL) {
	Var* next = var->next;

	if (strcmp(var->name, name) == 0) {
            /* if variable name was found, delete variable from chain */
	    if (last == NULL)
		*head = var->next;
	    else
		last->next = var->next;

            /* do the real work */
	    free(var->name);
	    free(var->value);
	    free(var);
	}
        else
	    last = var;

	var = next;
    }
}



/*
 * var_exit - recursivly free all allocated variables
 */
void
var_exit(Var** head)
{
    Var* var = *head;

    while(var != NULL) {
	Var* next = var->next;

	/* do the real work */
	free(var->name);
	free(var->value);
	free(var);

	var = next;
    }

    /* just to be shure */
    *head = NULL;
}
