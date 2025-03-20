#include <stdlib.h>
#include "graph.h"

void create_graph(graph_t **graph, unsigned int size, unsigned int capacity, unsigned int max_key_length) {
    (*graph) = calloc(1, sizeof(graph_t));
    (*graph)->adjacency_list = NULL;
    (*graph)->capacity = capacity;
    (*graph)->size = size;
    (*graph)->max_key_length = max_key_length;
    (*graph)->visited = calloc(size, sizeof(unsigned short));
    (*graph)->distances = calloc(size, sizeof(unsigned int));
}

void create_graph_adjacency_list(graph_t **graph, unsigned int first_field_offset, unsigned int first_field_datasize,
                        unsigned int second_field_offset, unsigned int second_field_datasize, unsigned int datasize, unsigned int keysize) {

    (*graph)->adjacency_list = calloc((*graph)->capacity, sizeof(list_t *));
    for (unsigned int i = 0; i < (*graph)->capacity; i++) {
        create_list(&(*graph)->adjacency_list[i], first_field_datasize, second_field_datasize, first_field_offset, second_field_offset, datasize, keysize, NULL);
    }
}

void add_entry(graph_t **graph, void *key, void *value) {
    for (unsigned int i = 0; i < (*graph)->capacity; i++) {
        if (!strcmp((char *)key, (*graph)->adjacency_list[i]->key)) {
            (*graph)->adjacency_list[i]->size++;
            add_node_to_back(2, &(*graph)->adjacency_list[i], value);

            return;
        }
    }
}

void remove_entry(graph_t **graph, void *key, void *data) {
    for (unsigned int i = 0; i < (*graph)->capacity; i++) {
        if (!strcmp((char *)(*graph)->adjacency_list[i]->key, (char *)key)) {
            (*graph)->adjacency_list[i]->size--;
            remove_node_given_key(2, data, &(*graph)->adjacency_list[i]);
        }
    }
}

void free_graph(graph_t **graph) {
    for (unsigned int i = 0; i < (*graph)->capacity; i++) {
        free_list(2, &(*graph)->adjacency_list[i]);
    }

    free((*graph)->visited);
    (*graph)->visited = NULL;
    free((*graph)->distances);
    (*graph)->distances = NULL;
    free((*graph)->adjacency_list);
    (*graph)->adjacency_list = NULL;

    free((*graph));
    (*graph) = NULL;
}

