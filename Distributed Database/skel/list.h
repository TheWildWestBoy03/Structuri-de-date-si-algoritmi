#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef struct list_node_t {
    void *data;
    struct list_node_t *next, *prev;
} list_node_t;

typedef struct list_t {
    int datasize;
    void *data;
    list_node_t *head, *tail;
} list_t;

void free_list(list_node_t **tail, list_node_t **head);
void add_node(void* data, list_node_t **head, list_node_t **tail, unsigned int datasize);
void remove_node(list_node_t **head, list_node_t **tail);
void create_list(unsigned int datasize, list_t **list, unsigned int keysize);
void create_list_array(list_t ***list_array, unsigned int arraycell_datasize, unsigned int maximum_size, unsigned int datasize);
list_node_t *return_oldest_node(list_t *list);
void free_list_skel(list_t **list);
void remove_node_with_key(list_t **list, void *key, unsigned int offset, unsigned int datasize);
void remove_node_from_head(list_t **list);
void add_node_in_ascending_order(list_t **list, void *key, unsigned int field_offset);
void free_list_normally(list_node_t **head);