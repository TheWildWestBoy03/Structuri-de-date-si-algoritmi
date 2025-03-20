#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

tree_node *create_node(void *data, unsigned int datasize) {
    (void)data;
    (void)datasize;
    tree_node *new_node = calloc(1, sizeof(tree_node));
    new_node->reposts = NULL;

    return new_node;
}

void create_tree(tree **new_tree, unsigned int datasize, unsigned int keysize, unsigned int tree_capacity) {
    (*new_tree) = calloc(1, sizeof(tree));
    (*new_tree)->datasize = datasize;
    (*new_tree)->keysize = keysize;
    (*new_tree)->tree_capacity = tree_capacity;
    (*new_tree)->tree_number_of_nodes = 0;

    (*new_tree)->tree_peaks = calloc(1, sizeof(tree_node));

    // the tree root has only one child, which is the main post root, leaving it empty
    (*new_tree)->tree_peaks[0] = calloc(1, sizeof(tree_node));
    (*new_tree)->tree_peaks[0]->reposts = NULL;
    (*new_tree)->tree_peaks[0]->data = calloc(1, datasize);
}

void *retrieve_node_via_key(unsigned int operation, unsigned int offset, unsigned int field_datasize, void *key, tree_node *root) {
    if (root == NULL) {
        return NULL;
    }

    if (operation == 0) {
        if (root->data) {
            if (!memcmp(((char *)root->data + offset), key, field_datasize)) {
                return root;
            }
        }
    } else {
        if (root->data) {
            if (memcmp(((char *)root->data + offset), key, field_datasize) > 0) {
                return root;
            }
        }
    }

    list_t *children = root->reposts;

    if (!children) {
        return NULL;
    }

    list_node_t *head = children->head;

    tree_node *next_candidate = NULL;
    while (head) {
        next_candidate = retrieve_node_via_key(operation, offset, field_datasize, key, head->data);

        if (next_candidate != NULL) {
            return next_candidate;
        }

        head = head->next;
    }

    return NULL;
}

tree_node *compute_lca(tree_node *root, unsigned int offset, unsigned int datasize, void *key1, void *key2) {
    if (!root) {
        return NULL;
    }

    void *data = root->data;
    
    if (data) {
        if (!memcmp((char *)data + offset, key1, datasize) || !memcmp((char *)data + offset, key2, datasize)) {
            return root;
        }
    }

    list_t *children = root->reposts;
    unsigned int lca_stopping_condition = 0;

    if (children) {
        list_node_t *head = children->head;
        while (head) {
            if (compute_lca(head->data, offset, datasize, key1, key2)) {
                lca_stopping_condition++;
            }

            if (lca_stopping_condition == 2) {
                return root;
            }
            head = head->next;
        }

        head = children->head;
        while (head) {
            if (compute_lca(head->data, offset, datasize, key1, key2)) {
                return compute_lca(head->data, offset, datasize, key1, key2);
            }
            head = head->next;
        }

        return NULL;
    } else {
        return NULL;
    }

    return NULL;
}

void delete_tree(tree_node **root, tree_node **tree_root) {
    if (*root) {
        list_t *children = (*root)->reposts;

        if (children) {
            list_node_t *head = children->head;
            while (head) {
                delete_tree((tree_node **)&head->data, tree_root);
                head = head->next;
            }

            if ((*root)->reposts) {
                list_node_t *head = (*root)->reposts->head;
                while (head) {
                    if (head->data) {
                        if (((tree_node *)head->data)->data) {
                            free(((tree_node *)head->data)->data);
                            ((tree_node *)head->data)->data = NULL;
                        }
                    }

                    head = head->next;
                }

                free_list(0, &(*root)->reposts);
            }

        }

        if ((*root)->data) {
            free((*root)->data);
            (*root)->data = NULL;
        }

        if (*root) {
            free(*root);
            (*root) = NULL;
        }
    }
}

void *retrieve_node_parent_via_key(unsigned int offset, unsigned int field_datasize, void *key, tree_node *root) {
    if (root == NULL) {
        return NULL;
    }

    list_t *children = root->reposts;

    if (!children) {
        return NULL;
    }

    list_node_t *head = children->head;

    tree_node *next_candidate = NULL;
    while (head) {
        
        if (head->data) {
            if (!memcmp((char *)((tree_node *)((char *)head->data))->data + offset, key, field_datasize)) {
                return root;
            }
        }
        
        next_candidate = retrieve_node_parent_via_key(offset, field_datasize, key, head->data);
        if (next_candidate != NULL) {
            return next_candidate;
        }

        head = head->next;
    }

    return NULL;
}