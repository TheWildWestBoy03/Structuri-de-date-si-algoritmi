/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity, int maximum_document_title_length, int maximum_document_content_length) {
    /* TODO */
    lru_cache *initialized_lru_cache = calloc(1, sizeof(lru_cache));
    
    initialized_lru_cache->cache_capacity = cache_capacity;
    create_list(sizeof(lru_cache_entry), &initialized_lru_cache->lru_cache_list, maximum_document_title_length);
 
    initialized_lru_cache->value_datasize = sizeof(lru_cache_entry);
    init_addresses_hashtable(&initialized_lru_cache->buckets, cache_capacity, maximum_document_title_length, initialized_lru_cache->value_datasize);
    initialized_lru_cache->hash_function_docs = hash_string;
    initialized_lru_cache->hash_function_servers = hash_uint;
    initialized_lru_cache->keysize = maximum_document_title_length;
    initialized_lru_cache->document_content_length = maximum_document_content_length;

    return initialized_lru_cache;
}

void init_addresses_hashtable(list_t ***addresses_hashtable, unsigned int cache_capacity, int maximum_document_title_length, unsigned int datasize) {
    create_list_array(addresses_hashtable, maximum_document_title_length, cache_capacity, datasize);
}


bool lru_cache_is_full(lru_cache *cache) {
    /* TODO */
    return cache->cache_capacity == cache->size;
}

void free_lru_cache(lru_cache **cache) {
    /* TODO */

    (*cache)->hash_function_docs = NULL;
    (*cache)->hash_function_servers = NULL;

    free_internal_structure_strings(&(*cache)->lru_cache_list->head);

    free_list_normally(&(*cache)->lru_cache_list->head);
    free((*cache)->lru_cache_list->data);
    (*cache)->lru_cache_list->data = NULL;

    free((*cache)->lru_cache_list);
    (*cache)->lru_cache_list = NULL;

    for (unsigned int i = 0; i < (*cache)->cache_capacity; i++) {
        free_internal_structure_strings(&(*cache)->buckets[i]->head);
        free_list_normally(&(*cache)->buckets[i]->head);

        (*cache)->buckets[i]->head = NULL;
        (*cache)->buckets[i]->tail = NULL;

        free((*cache)->buckets[i]->data);
        (*cache)->buckets[i]->data = NULL;

        free((*cache)->buckets[i]);
    }

    free((*cache)->buckets);
    (*cache)->buckets = NULL;

    free((*cache));
    (*cache) = NULL;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key) {

    /* TODO */
    unsigned int index_to_use = cache->hash_function_docs(key) % cache->cache_capacity;
    add_node(value, &(cache->buckets[index_to_use]->head), &cache->buckets[index_to_use]->tail, cache->buckets[index_to_use]->datasize);

    // i have to create a copy of the value element
    void *copied_value = calloc(1, cache->buckets[index_to_use]->datasize);
    ((lru_cache_entry *)copied_value)->document_name = calloc(1, cache->keysize);
    ((lru_cache_entry *)copied_value)->document_content = calloc(1, cache->document_content_length);
    memcpy(((lru_cache_entry *)copied_value)->document_content, ((lru_cache_entry *)value)->document_content, cache->document_content_length);
    memcpy(((lru_cache_entry *)copied_value)->document_name, ((lru_cache_entry *)value)->document_name, cache->keysize);

    add_node(copied_value, &cache->lru_cache_list->head, &cache->lru_cache_list->tail, cache->lru_cache_list->datasize);

    ((lru_cache_entry *)((cache->lru_cache_list->head->data)))->address = cache->lru_cache_list->head;
    ((lru_cache_entry *)((cache->buckets[index_to_use]->head->data)))->address = cache->lru_cache_list->head;
    
    free(copied_value);
    
    void *address = evicted_key ? *evicted_key : NULL;
    
    if (!address) {
        cache->size++;

        return false;
    }

    lru_cache_remove(cache, *evicted_key);

    free(((lru_cache_entry *)(cache->lru_cache_list->tail->data))->document_content);
    ((lru_cache_entry *)(cache->lru_cache_list->tail->data))->document_content = NULL;
    free(((lru_cache_entry *)(cache->lru_cache_list->tail->data))->document_name);
    ((lru_cache_entry *)(cache->lru_cache_list->tail->data))->document_name = NULL;
   
    remove_node(&cache->lru_cache_list->head, &cache->lru_cache_list->tail);

    return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
    /* TODO */
    unsigned int index_to_check = cache->hash_function_docs(key) % cache->cache_capacity;

    list_t *list_to_check = cache->buckets[index_to_check];

    if (list_to_check == NULL) {
        return NULL;
    }

    list_node_t *head = list_to_check->head;

    while (head != NULL) {
        if (head->data) {
            if (!strcmp((char *) key, ((lru_cache_entry *)(head->data))->document_name)) {
                return head->data;
            }
        }
        head = head->next;
    }

    return NULL;
}

void lru_cache_remove(lru_cache *cache, void *key) {
    /* TODO */
    unsigned int index_to_use = cache->hash_function_docs(key) % cache->cache_capacity;
    list_t *key_bucket = cache->buckets[index_to_use];
    list_node_t *head = key_bucket->head;

    while (head) {
        if (!strcmp((char *)key, ((lru_cache_entry *)(head->data))->document_name)) {
            list_node_t *deletable = head;

            // this is the document we have to delete from cache
            free(((lru_cache_entry *)(deletable->data))->document_name);
            free(((lru_cache_entry *)(deletable->data))->document_content);

            ((lru_cache_entry *)(deletable->data))->document_name = NULL;
            ((lru_cache_entry *)(deletable->data))->document_content = NULL;

            if (!head->prev) {
                key_bucket->head = key_bucket->head->next;
                
                if (!key_bucket->head) {
                    key_bucket->tail = NULL;
                } else {
                    key_bucket->head->prev = NULL;
                }
            } else {
                if (!head->next) {
                    key_bucket->tail = key_bucket->tail->prev;
                    if (key_bucket->tail) {
                        key_bucket->tail->next = NULL;
                    }
                } else {
                    head->next->prev = head->prev;
                    head->prev->next = head->next;
                }
            }

            free(deletable->data);
            deletable->data = NULL;

            free(deletable);
            deletable = NULL;

            return;
        }

        head = head->next;
    }
}

void *find_evicted_key(lru_cache *cache) {
    if (cache->lru_cache_list->tail) {
        return cache->lru_cache_list->tail->data;
    }

    return NULL;
}


void free_internal_structure_strings(list_node_t **head) {
    list_node_t *head_copy = *head;

    while (head_copy) {
        free(((lru_cache_entry *)head_copy->data)->document_content);
        ((lru_cache_entry *)head_copy->data)->document_content = NULL;
        free(((lru_cache_entry *)head_copy->data)->document_name);
        ((lru_cache_entry *)head_copy->data)->document_name = NULL;

        head_copy = head_copy->next;
    }
}

void print_cache(lru_cache *cache) {
    for (unsigned int i = 0; i < cache->cache_capacity; i++) {
        list_node_t *head = cache->buckets[i]->head;

        if (head) {
            printf("cache list number %d ----- ", i);
            
            while (head) {
                printf("%s - %s ", ((lru_cache_entry *)head->data)->document_name, ((lru_cache_entry *)head->data)->document_content);

                head = head->next;
            }

            printf("\n");
        }
    }

    list_node_t *head = cache->lru_cache_list->head;

    printf("The current queue size is: %d \n", cache->size);
    while (head) {
        printf("%s - %s ", ((lru_cache_entry *)head->data)->document_name, ((lru_cache_entry *)head->data)->document_content);
        head = head->next;
    }

    list_node_t *tail = cache->lru_cache_list->tail;
    printf("The current queue size is: %d \n", cache->size);
    while (tail) {
        printf("%s - %s ", ((lru_cache_entry *)tail->data)->document_name, ((lru_cache_entry *)tail->data)->document_content);
        tail = tail->prev;
    }

    printf("\n");
} 