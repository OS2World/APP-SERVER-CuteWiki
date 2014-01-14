/*
 * page_list.h - all things done with lists of pages
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef PAGE_LIST_H
#define PAGE_LIST_H

#include "types.h"



extern char* pagepath;

void	 	pagelist_init(const char* pathname);
void	 	pagelist_exit();

Page *          pagelist_insert_page(const char* name, int flags);
Page * 		pagelist_find_page(const char* title);
bool		pagelist_remove_page(const char* name);

size_t		pagelist_get_count();
size_t	 	pagelist_get_usedmemory();
size_t		pagelist_get_useddisk();

Page**  	pagelist();
Page**		pagelist_alpha_sorted();
Page**		pagelist_time_sorted();

Page**		pagelist_search_topic(const char* search);
Page**          pagelist_search_title(const char* search, const char* category);
Page**          pagelist_search_full(const char* search, const char* category);

Page**		pagelist_of_reverse_links(const char* name);
Page**          pagelist_in_category(const char* name);

Page**          pagelist_get_users();
Page**          pagelist_get_groups();
Page**          pagelist_get_categories();



#endif
