#include "list.h"

#ifndef DATABASE_H
#define DATABASE_H

typedef struct database_entry {
    char *document_name;
    char *document_content;
    unsigned int index;
} database_entry;

typedef struct database {
    list_t **hashtable;
    int size;
    int capacity;
    unsigned int database_key_datasize;
    char *document_name_key;
    unsigned int (*hash_function_servers)(void *);
    unsigned int (*hash_function_docs)(void *);
} database;

database *init_database(unsigned int max_key_length, unsigned int max_database_capacity, unsigned int (*hash_string)(void *), unsigned int (*hash_uint)(void *));
void free_database(database **database);
void *database_get_entry(database *database, void *key);

#endif