#ifndef FEED_H
#define FEED_H

#include "friends.h"
#include "posts.h"
#include "tree.h"
/**
 * Function that handles the calling of every command from task 3
 *
 * Please add any necessary parameters to the functions
*/
void handle_input_feed(char *input, posts_meta **post_meta, friends **friends_network);
void compute_user_feed(posts_meta **post_meta, friends **friends_network, char *username, unsigned int size);
void compute_user_profile(posts_meta **post_meta, char *username);
void print_user_reposts(tree_node *root, tree_node *tree_root, char *owner, char *title);
int return_friend_reposting_statement(tree_node *root, tree_node *tree_root, unsigned int user_id);
void compute_friends_reposts(posts_meta **post_meta, friends **friends_network, char *username, unsigned int post_id);

#endif // FEED_H
