/*
 * Copyright (c) 2024, <>
 */
#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdbool.h>
#include "list.h"

typedef struct lru_cache_entry {
    char *document_name;
    char *document_content;
    void *address;
} lru_cache_entry;

typedef struct lru_cache {
    /* TODO */
    // the list used for ordering cache entries
    list_t *lru_cache_list;

    // the structure of key-value pair lists, where the key is the document name, while the value is the lru_cache_entry structure
    list_t **buckets;

    // cache maximum size, specific to server
    unsigned int cache_capacity;

    // the current number of elements in cache
    unsigned int size;

    // here is the value datasize, which is sizeof(lru_cache_entry)
    unsigned int value_datasize; 

    // this is the size of the key
    unsigned int keysize;

    // the maximum size of the document content
    unsigned int document_content_length;
    // useful hashing functions
    unsigned int (*hash_function_servers)(void *);
    unsigned int (*hash_function_docs)(void *);
} lru_cache;

lru_cache *init_lru_cache(unsigned int cache_capacity, int maximum_document_title_length, int maximum_document_content_length);

bool lru_cache_is_full(lru_cache *cache);

void free_lru_cache(lru_cache **cache);

/**
 * lru_cache_put() - Adds a new pair in our cache.
 * 
 * @param cache: Cache where the key-value pair will be stored.
 * @param key: Key of the pair.
 * @param value: Value of the pair.
 * @param evicted_key: The function will RETURN via this parameter the
 *      key removed from cache if the cache was full.
 * 
 * @return - true if the key was added to the cache,
 *      false if the key already existed.
 */
bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key);

/**
 * lru_cache_get() - Retrieves the value associated with a key.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
 * 
 * @return - The value associated with the key,
 *      or NULL if the key is not found.
 */
void *lru_cache_get(lru_cache *cache, void *key);

/**
 * lru_cache_remove() - Removes a key-value pair from the cache.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
*/
void lru_cache_remove(lru_cache *cache, void *key);

void init_addresses_hashtable(list_t ***addresses_hashtable, unsigned int cache_capacity,
                 int maximum_document_title_length, unsigned int datasize);

void *find_evicted_key(lru_cache *cache);
void free_internal_structure_strings(list_node_t **head);
void print_cache(lru_cache *cache);

#endif /* LRU_CACHE_H */
