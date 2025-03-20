#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "constants.h"

void free_list(list_node_t **head, list_node_t **tail) {
    if (!(*head)) {
        return;
    }

    while ((*tail) != NULL) {
        remove_node(head, tail);
    }

    (*head) = NULL;
}

void free_list_skel(list_t **list) {
    if (!(*list)) return;

    free_list(&(*list)->head, &(*list)->tail);
    
    free((*list)->data);
    free(*list);
    *list = NULL;
}


void add_node(void* data, list_node_t **head, list_node_t **tail, unsigned int datasize) {
    list_node_t *new_node = malloc(sizeof(list_node_t));
    new_node->next = NULL;
    new_node->prev = NULL;

    new_node->data = malloc(datasize);
    memcpy(new_node->data, data, datasize);

    if (!(*head)) {
        (*head) = new_node;
        (*tail) = new_node;
        return;
    }

    new_node->next = (*head);
    (*head)->prev = new_node;
    (*head) = new_node;
}

void remove_node(list_node_t **head, list_node_t **tail) {
    if (!(*tail)) {
        (*head) = NULL;
        return;
    }

    if (!(*tail)->next && !(*tail)->prev) {
        free((*tail)->data);
        (*tail)->data = NULL;

        free((*tail));
        (*tail) = NULL;
        (*head) = NULL;
        
        return;
    }

    list_node_t *node_to_delete = *tail;
    (*tail)->next = NULL;
    
    *tail = (*tail)->prev;

    if (*tail) {
        (*tail)->next = NULL;
    }
    
    free(node_to_delete->data);
    node_to_delete->data = NULL;
    
    free(node_to_delete);
    node_to_delete = NULL;
}

void create_list(unsigned int datasize, list_t **list, unsigned int keysize) {
    (*list) = calloc(1, sizeof(list_t));
    (*list)->datasize = datasize;
    (*list)->head = (*list)->tail = NULL;
    (*list)->data = calloc(1, keysize + 1);
}

void create_list_array(list_t ***list_array, unsigned int arraycell_datasize, unsigned int maximum_size, unsigned int datasize) {
    (*list_array) = malloc(sizeof(list_t) * maximum_size);

    for (unsigned int i = 0; i < maximum_size; i++) {
        create_list(datasize, &(*list_array)[i], arraycell_datasize);
        (*list_array)[i]->head = (*list_array)[i]->tail = NULL;
    }
}

list_node_t *return_oldest_node(list_t *list) {
    return list->tail;
}

void free_list_normally(list_node_t **head) {
    list_node_t *copy = *head;

    int index = 0;
    while(copy) {
        index++;
        list_node_t *delete = copy;
        copy = copy->next;

        free((delete->data));
        delete->data = NULL;

        free(delete);
        delete = NULL;
    }

    (*head) = NULL;
}

void remove_node_from_head(list_t **list) {
    list_node_t *delete = (*list)->head;
    (*list)->head = (*list)->head->next;

    free(delete->data);
    delete->data = NULL;

    free(delete);
    delete = NULL;
}

void remove_node_with_key(list_t **list, void *key, unsigned int offset, unsigned int datasize) {
    if (!(*list)) {
        return;
    }
    list_node_t *head = (*list)->head;

    while (head) {
        if (!memcmp(key, head->data, datasize)) {
            if (!head->prev && !head->next) {
                char *key = *(char **)((char *)head->data + offset);
                free(key);
                free(head->data);
                head->data = NULL;

                free(head);
                head = NULL;
                
                return;
            }
            if (!head->next) {
                list_node_t *delete = head;

                head->prev->next = NULL;
                delete->prev = NULL;

                char *key = *(char **)((char *)delete->data + offset);
                free(key);
                free(delete->data);
                delete->data = NULL;

                free(delete);
                delete = NULL;
            } else if (!head->prev) {
                list_node_t *delete = head;

                (*list)->head = (*list)->head->next;
                (*list)->head->prev = NULL;

                head = head->next;
                char *key = *(char **)((char *)delete->data + offset);
                free(key);

                free(delete->data);
                delete->data = NULL;

                free(delete);
                delete = NULL;
            } else {
                list_node_t *delete = head;

                head->prev->next = head->next;
                head->next->prev = head->prev;

                head = head->next;
                char *key = *(char **)((char *)delete->data + offset);
                free(key);

                free(delete->data);
                delete->data = NULL;

                free(delete);
                delete = NULL;
            }

            return;
        }

        head = head->next;
    }
}

void add_node_in_ascending_order(list_t **list, void *key, unsigned int field_offset) {
    list_node_t *new_node = calloc(1, sizeof(list_node_t));
    list_node_t *tail = NULL;
    new_node->data = calloc(1, (*list)->datasize);
    memcpy(new_node->data, key, (*list)->datasize);

    if (!(*list)) {
        return;
    }

    if (!(*list)->head) {
        (*list)->head = new_node;

        return;
    }

    list_node_t *head = (*list)->head;

    while (head) {
        char *key1 = *(char **)((char *)head->data + field_offset);
        char *key2 = *(char **)((char *)key + field_offset);
        
        unsigned int numberized_key1 = atoi(key1);
        unsigned int numberized_key2 = atoi(key2);
        
        if (numberized_key1 > numberized_key2)  {
            break;
        } else if (numberized_key1 == numberized_key2) {
            unsigned int server_id_1 = *(int *)(head->data);
            unsigned int server_id_2 = *(int *)(key);

            if (server_id_1 > server_id_2) {
                break;
            }
        }

        tail = head;
        head = head->next;
    }
    
    if (head) {
        new_node->prev = head->prev;
        new_node->next = head;
        
        if (head->prev) {
            head->prev->next = new_node;
            head->prev = new_node;
        } else {
            head->prev = new_node;
            head = new_node;
            (*list)->head = new_node;
        }
        
    } else {
        tail->next = new_node;
        new_node->prev = tail;
    }
}