/*
 * list.c - linked list routines
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

#include "list.h"



/*
 * list_new - create a new list head
 */
static struct item*
list_new_item(char *name, void *data)
{
    struct item* list;


    list = malloc(sizeof(struct item));
    memset(list, 0, sizeof(struct item));

    printf("list new: %p\n", list);
    if (name != NULL)
	list->name = strdup(name);
    list->data = data;
    list->next = NULL;

    return list;
}

/*
 * list_del - delete the whole list
 *
 * Beware: just use this, if alloced data don't need to be freed!
 */
void
list_del(struct item** head)
{
    struct item * cur;

    printf("list del: %p\n", *head);
    cur = *head;
    while (cur != NULL)
    {
	struct item * next;

	next = cur->next;
	if (cur->name != NULL)
	    free (cur->name);
	free(cur);
	cur = next;
    }
    *head = NULL;
}

/*
 * list_add_item - insert a new list item
 */
void
list_add_item(struct item** head, char *name, void *data)
{
    struct item* current;

    if (name == NULL)
	return;

    //assert(head == NULL);
    printf("list add1: %p\n", *head);
    if (*head == NULL)
    {

	*head = list_new_item(name, data);
	return;
    }
    printf("list add2: %p\n", *head);

    /* find end of list and insert item */
    current = *head;
    while (current->next != NULL)
	current = current->next;
    current->next = list_new_item(name, data);
}

void *
list_del_item(struct item** head, char *name)
{
    struct item* current;

    if (name == NULL)
	return NULL;

    printf("list del: %p\n", *head);
    for (current = *head; current != NULL; current = current->next)
    {
	if (current->name && !strcmp(current->name, name))
	{
	    void * data;

	    if (current == *head)
                *head = current->next;

	    data = current->data;
	    free (current->name);
	    free(current);

	    return data;
	}
    }
    printf("list del: %p\n", *head);

    return NULL;
}

/*
 * list_get_item - fetch a list item
 */
void *
list_get_item(struct item* head, char *name)
{
    struct item* current;

    printf("list get item: %p\n", head);
    if (name == NULL)
	return NULL;

    for (current = head; current != NULL; current = current->next)
    {
	if (current->name && !strcmp(current->name, name))
	    return current->data;
    }

    return NULL;
}
