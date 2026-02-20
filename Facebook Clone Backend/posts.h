#ifndef POSTS_H
#define POSTS_H

#include "tree.h"
#define MAX_POSTS 200
#define MAX_COMMAND_LEN 500
#define MAX_TITLE_LENGTH 280

typedef struct repost {
    unsigned int repost_timeprint;
    unsigned int repost_id;
    unsigned int user_id;
    unsigned int number_of_likes;
} repost;

typedef struct post {
    unsigned int post_id;
    unsigned int user_id;
    char *title;
    char *owner;
    unsigned int pending_repost_id;
    unsigned int number_of_likes; 
    tree *recurrent_tree;
} post;

typedef struct posts_meta {
    post **post_list;
    unsigned int size;
    unsigned int pending_post_id;
    unsigned int capacity;
    unsigned int maximum_users;
    int **appreciations_matrix;
} posts_meta;

/**
 * Function that handles the calling of every command from task 2
 *
 * Please add any necessary parameters to the functions
*/
repost *allocate_new_repost(unsigned int repost_id, unsigned int user_id);
void handle_input_posts(char *input, posts_meta **post_meta);
void initialize_liking_user_post_matrix(int ***matrix, unsigned int maximum_users);
void initialize_posts_array(posts_meta **post_meta, unsigned int maxsize, unsigned int maximum_users);
void create_post(posts_meta **post_meta, char *owner, char *title);
void create_repost(posts_meta **posts_meta, char *owner, unsigned int parent_post_id, unsigned int parent_repost_id);
void like(posts_meta **posts_meta, char *name, unsigned int post_id, unsigned int repost_id);
void compute_ratio(posts_meta **posts_meta, unsigned int post_id);
void get_likes(posts_meta **posts_meta, unsigned int post_id, unsigned int repost_id);
void print_reposts(tree_node *root, tree_node *tree_root);
void get_reposts(posts_meta **posts_meta, unsigned int post_id, unsigned int repost_id);
void create_post_element(posts_meta **post_meta, post **post, char *owner, char *title);
void delete_entity(posts_meta **post_meta, unsigned int post_id, unsigned int repost_id);
void compute_common_repost(posts_meta **post_meta, unsigned int post_id, unsigned int first_id, unsigned int second_id);
void free_structures(posts_meta **post_meta);

#endif // POSTS_H
