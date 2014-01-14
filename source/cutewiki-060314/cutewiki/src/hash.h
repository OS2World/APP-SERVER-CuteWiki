/*
 * hash.c - hash routines
 *
 * Copyright 2003, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef HASH_H
#define HASH_H

typedef unsigned int (*HashFunc) (const char* key);
typedef bool (*EqualFunc) (const void* self, const void* other);

typedef struct _HashNode HashNode;
typedef struct _Hash Hash;


/* 
 *Hash tables
 */
Hash* 	hash_new ();
void 	hash_del (Hash *hash);
void ** hash_get_list (Hash *);
void ** hash_get_sorted_list (Hash *, CompFunc);
void* 	hash_find (Hash *hash, const char* key);
void 	hash_insert (Hash *hash, const char* key, void* value);
bool 	hash_remove (Hash *hash, const char* key);
void 	hash_foreach(Hash *hash, SetFunc, void*);
int 	hash_checkall(Hash *hash, CheckFunc);
int 	hash_query(Hash *, CompFunc, ExecFunc, void *);
size_t  hash_get_size (Hash *hash);



#endif
