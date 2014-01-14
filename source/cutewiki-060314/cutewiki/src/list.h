/*
 * list.h - linked list routines
 *
 * Copyright 2003, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef LIST_H
#define LIST_H


struct item
{
    char * name;
    void * data;
    struct item * next;
};

struct item* list_new(char *name, void *data);
void list_del(struct item** head);

void list_add_item(struct item** head, char *name, void *data);
void* list_del_item(struct item** head, char *name);
void*  list_get_item(struct item* head, char *name);


#endif
