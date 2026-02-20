#ifndef FRIENDS_H
#define FRIENDS_H

#define MAX_COMMAND_LEN 500
#define MAX_PEOPLE 550

#include "graph.h"
#include <sys/types.h>

typedef struct friends {
    unsigned int size;
    graph_t *graph;
} friends;

typedef struct friend_info {
    unsigned int friend_id;
    char *friend_name;
} friend_info;

/**
 * Function that handles the calling of every command from task 1
 *
 * Please add any necessary parameters to the functions
*/

u_int16_t retrieve_number_of_users(u_int16_t id);
friends *init_friends_network(unsigned int total_number_of_users);
void handle_input_friends(char *input, friends **friends_network_main);
void add_connection(friends **friends, void *key1, void *key2);
void remove_connection(friends **friends, void *key1, void *key2);
void compute_suggestions(friends *friends, void *key);
void compute_distance(friends *friends, void *key1, void *key2);
void compute_common_friends(friends *friends, void *key1, void *key2);
void display_friends(friends *friends, void *key1);
void compute_popular(friends *friends, void *key1);
void fill_graph_keys(graph_t **graph, unsigned int size);
void* is_my_friend(friends *friends, void *key1, void *key2);

#endif // FRIENDS_H
