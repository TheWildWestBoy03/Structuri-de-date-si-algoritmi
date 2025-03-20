#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct list_node_t {
    void *data;
    struct list_node_t *next, *prev;
} list_node_t;

typedef struct list_t {
    unsigned int first_field_datasize;
    unsigned int second_field_datasize;
    unsigned int first_field_offset;
    unsigned int second_field_offset;
    unsigned int size;
    void *key;
    unsigned int datasize;
    unsigned int keysize;   
    list_node_t *head, *tail;
} list_t;

void add_node_to_back(unsigned int field_order, list_t **list, void *data);
void add_node_to_front(unsigned int field_order, list_t **list, void *data);
void create_list(list_t **list, unsigned int first_field_datasize, unsigned int second_field_datasize, unsigned int first_field_offset,
    unsigned int second_field_offset, unsigned int datasize, unsigned int keysize, void *key);
void free_list(unsigned int field_order, list_t **list);
void remove_node_given_key(unsigned int field_order, void* key, list_t **list);
void remove_node_from_front(unsigned int field_order, list_t **list);
int get_item(unsigned int field_order, list_t **list, void *key);

#endif /*LIST_H*/