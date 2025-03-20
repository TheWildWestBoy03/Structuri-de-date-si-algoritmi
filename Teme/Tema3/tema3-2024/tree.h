#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "list.h"

typedef struct tree_node {
    void *data;
    list_t *reposts;
} tree_node;

typedef struct tree {
    unsigned int datasize;
    unsigned int keysize;
    unsigned int tree_number_of_nodes;
    unsigned int tree_capacity;
    tree_node **tree_peaks;
} tree;

void create_tree(tree **new_tree, unsigned int datasize, unsigned int keysize, unsigned int tree_capacity);
void *retrieve_node_via_key(unsigned int operation, unsigned int offset, unsigned int field_datasize, void *key, tree_node *root);
tree_node *create_node(void *data, unsigned int datasize);
tree_node *compute_lca(tree_node *root, unsigned int offset, unsigned int datasize, void *key1, void *key2);
void delete_tree(tree_node **root, tree_node **tree_root);
void *retrieve_node_parent_via_key(unsigned int offset, unsigned int field_datasize, void *key, tree_node *root);

#endif