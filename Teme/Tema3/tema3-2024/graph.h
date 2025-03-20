#include <stdlib.h>
#include "list.h"

typedef struct graph_t {
    list_t **adjacency_list;
    unsigned int size;
    unsigned int capacity;
    unsigned int max_key_length;
    unsigned short *visited;
    unsigned int *distances;
} graph_t;

void create_graph(graph_t **graph, unsigned int size, unsigned int capacity, unsigned int max_key_length);
void create_graph_adjacency_list(graph_t **graph, unsigned int first_field_offset, unsigned int first_field_datasize,
            unsigned int second_field_offset, unsigned int second_field_datasize, unsigned int datasize, unsigned int keysize);

void add_entry(graph_t **graph, void *key, void *value);
void remove_entry(graph_t **graph, void *key, void *value);
void free_graph(graph_t **graph);
void print_bfs_level(graph_t *graph, unsigned int level);
