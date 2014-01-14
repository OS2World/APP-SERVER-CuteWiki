/*
 * hash.c - hash routines
 *
 * Copyright 2001 Xiong PuXiang
 * Copyright 2003, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "cutewiki.h"
#include "page.h"
#include "hash.h"

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163

struct _HashNode
{
    const char* key;
    void* value;
    HashNode *next;
};

struct _Hash
{
    unsigned int size;
    unsigned int nnodes;
    HashNode **nodes;
    HashFunc hash_func;
    EqualFunc key_equal_func;
};


static const unsigned int primes[] =
  {
    11,
    19,
    37,
    73,
    109,
    163,
    251,
    367,
    557,
    823,
    1237,
    1861,
    2777,
    4177,
    6247,
    9371,
    14057,
    21089,
    31627,
    47431,
    71143,
    106721,
    160073,
    240101,
    360163,
    540217,
    810343,
    1215497,
    1823231,
    2734867,
    4102283,
    6153409,
    9230113,
    13845163,
  };

static const unsigned int nprimes = sizeof (primes) / sizeof (primes[0]);

/* static functions */
static void hash_resize (Hash *hash);
static HashNode** hash_find_node (Hash *hash, const char* key);
static HashNode* hashnode_new (const char* key, void* value);
static void hashnode_del (HashNode *hash_node);
static unsigned int primes_closest (unsigned int num);
static void hash_need_resize(Hash *hash);


static bool
string_equal(void * value1, void * value2)
{
    return strcmp((char *)value1, (char *)value2) == 0 ? true:false;
}

static uint
string_hash(void * value)
{
    const char *p;
    int h=0, g;

    for(p = (char *)value; *p != '\0'; p += 1) {
        h = ( h << 4 ) + *p;
        if ( ( g = h & 0xf0000000 ) ) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h ;
}


static Hash*
hash_create_strhash (HashFunc hash_func, EqualFunc key_equal_func)
{
    Hash *hash;
    unsigned int i;

    hash = malloc( sizeof(Hash) );

    hash->size               = HASH_TABLE_MIN_SIZE;
    hash->nnodes             = 0;
    hash->hash_func          = hash_func;
    hash->key_equal_func     = key_equal_func;
    hash->nodes              = calloc (hash->size, sizeof(HashNode));

    for (i = 0; i < hash->size; i++)
        hash->nodes[i] = NULL;

    return hash;
}

Hash*
hash_new ()
{
    return hash_create_strhash((HashFunc)string_hash, (EqualFunc)string_equal);
}

void
hash_del (Hash *hash)
{
    unsigned int i;

    if ( hash == NULL )
        return;

    for (i = 0; i < hash->size; i++) {
        HashNode *node = hash->nodes[i];
        while (node) {
            HashNode *next = node->next;
            hashnode_del(node);
            node = next;
        }
    }

    free (hash->nodes);
    free (hash);
}

/*
 * get the first valid entry
 */
void *
hash_first (Hash *hash)
{
    unsigned int i;

    if ( hash == NULL )
        return NULL;

    for (i = 0; i < hash->size; i++) {
        HashNode *node = hash->nodes[i];
        if (node) {
            return node->value;
        }
    }

    return NULL;
}

static HashNode**
hash_find_node (Hash *hash_table, const char* key)
{
    HashNode **node;

    node = &hash_table->nodes [(* hash_table->hash_func) (key) % hash_table->size];

    if (hash_table->key_equal_func)
        while (*node && !(*hash_table->key_equal_func) ((*node)->key, key))
            node = &(*node)->next;
    else
        while (*node && (*node)->key != key)
            node = &(*node)->next;

    return node;
}

void*
hash_find (Hash *hash, const char* key)
{
    HashNode *node;
    if ( hash == NULL || key == NULL)
        return NULL;

    node = *hash_find_node (hash, key);
    if (node)
        return node->value;
    else
        return NULL;
}

void
hash_insert (Hash *hash, const char* key, void* value)
{
    HashNode** node;

    if (hash == NULL)
        return;

    node = hash_find_node (hash, key);

    if (*node) {
        (*node)->value = value;
    }
    else {
        *node = hashnode_new (key, value);
        hash->nnodes++;
        hash_need_resize (hash);
    }
}

bool
hash_remove (Hash *hash, const char*  key)
{
    HashNode **node, *dest;

    if (hash == NULL)
        return false;

    node = hash_find_node (hash, key);
    if (*node) {
        dest = *node;
        *node = dest->next;
        hashnode_del (dest);
        hash->nnodes--;
        hash_need_resize (hash);

        return true;
    }

    return false;
}


/*
 * Get a NULL terminated list of all stored values
 *
 * The additional entry is for to hold a stopping NULL pointer
 */
void **
hash_get_list (Hash *hash )
{
    void**	list;
    void**	ptr;
    unsigned int i;
    HashNode *node;
    
    if (hash == NULL)
        return NULL;

    list = calloc(hash->nnodes + 1, sizeof(void*) );
    ptr = list;
    for (i = 0; i < hash->size; i++) {
	for (node = hash->nodes[i]; node; node = node->next) {
            *ptr++ = node->value;
	}
    }
    *ptr = NULL;        /* terminator */

    return list;
}

/*
 * Get a list of values, which fullfill get_func and sort them with compare
 */
void **
hash_get_sorted_list (Hash *hash, CompFunc compare)
{
    void** list;
    
    if (hash == NULL)
        return NULL;

    list = hash_get_list(hash);
    qsort(list, hash->nnodes, sizeof(void*), compare);

    return list;
}

size_t
hash_get_size (Hash *hash)
{
    if (hash == NULL)
        return 0;

    return hash->nnodes;
}

static void
hash_need_resize(Hash *hash)
{
    if ((hash->size >= 3*hash->nnodes && hash->size > HASH_TABLE_MIN_SIZE) ||
        (3 * hash->size <= hash->nnodes && hash->size < HASH_TABLE_MAX_SIZE))
        hash_resize (hash);
}

static void
hash_resize (Hash *hash)
{
    HashNode **new_nodes;
    HashNode *node;
    HashNode *next;
    unsigned int hash_val;
    int new_size;
    unsigned int i;

    i = primes_closest(hash->nnodes);
    new_size = i > HASH_TABLE_MAX_SIZE ? HASH_TABLE_MAX_SIZE : i < HASH_TABLE_MIN_SIZE ? HASH_TABLE_MIN_SIZE : i ;

    new_nodes = calloc ( new_size, sizeof(HashNode) );

    for (i = 0; i < hash->size; i++)
        for (node = hash->nodes[i]; node; node = next) {
            next = node->next;

            hash_val = (* hash->hash_func) (node->key) % new_size;

            node->next = new_nodes[hash_val];
            new_nodes[hash_val] = node;
        }

    free(hash->nodes);
    hash->nodes = new_nodes;
    hash->size = new_size;
}

static HashNode*
hashnode_new (const char* key, void* value)
{
    HashNode *hash_node;

    hash_node = malloc ( sizeof(HashNode) );
    hash_node->key = key;
    hash_node->value = value;
    hash_node->next = NULL;

    return hash_node;
}

static void
hashnode_del (HashNode *hash_node)
{
    hash_node->key = NULL;
    hash_node->value = NULL;
    free(hash_node);
}

static unsigned int
primes_closest (unsigned int num)
{
    unsigned int i;

    for (i = 0; i < nprimes; i++)
        if (primes[i] > num)
            return primes[i];

    return primes[nprimes - 1];
}



#define HASH_TEST 0
#if HASH_TEST

int
hash_checkall(Hash *hash, CheckFunc check)
{
    HashNode *node;
    unsigned int i;
    int count = 0;

    if (hash == NULL || check == NULL)
        return 0;

    for (i = 0; i < hash->size; i++)
        for (node = hash->nodes[i]; node; node = node->next)
            if ( (* check) (node->value) )
                count++;

    return count;
}

/* Hash test functions (see above) */

bool
string_equal(void * value1, void * value2)
{
    return strcmp((char *)value1, (char *)value2) == 0 ? true:false;
}

uint
string_hash(void * value)
{
    const char *p;
    int h=0, g;

    for(p = (char *)value; *p != '\0'; p += 1) {
        h = ( h << 4 ) + *p;
        if ( ( g = h & 0xf0000000 ) ) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h ;
}

void
string_print(void *value)
{
    printf("%s\n", (char *)value);
}



int
main(void)
{
    Hash *table;
    int i;
    void *j;

    char *strings[] = {
        "The first string",
        "The second string",
        "The third string",
        "The fourth string",
        "A much longer string than the rest in this example.",
        "The last string",
        NULL
    };
    char *junk[] = {
        "The first data",
        "The second data",
        "The third data",
        "The fourth data",
        "The fifth datum",
        "The sixth piece of data"
    };

    printf("=== Create hash table!\n");
    table = hash_new(string_hash, string_equal);

    printf("=== Find on empty hash table!\n");
    for (i=0;NULL != strings[i];i++) {
        j = hash_find(table, strings[i]);
        if (NULL == j)
            printf("\n'%s' is not in table",strings[i]);
        else  printf("\nERROR: %s was deleted but is still in table.",
                     strings[i]);
    }

    printf("=== Do the inserts!\n");
    for (i = 0; NULL != strings[i]; i++ )
        hash_insert (table, (void *)strings[i], (void *)junk[i]);

    for (i=0;NULL != strings[i];i++) {
        printf("\n%s = ", strings[i]);
        hash_checkall(table, (CheckFunc)string_print);
        hash_remove (table, (void *)strings[i]);
    }

    for (i=0;NULL != strings[i];i++) {
        j = hash_find(table, strings[i]);
        if (NULL == j)
            printf("\n'%s' is not in table",strings[i]);
        else  printf("\nERROR: %s was deleted but is still in table.",
                     strings[i]);
    }

    hash_destroy(table);
    return 0;
}



#endif /* HASH_TEST */

