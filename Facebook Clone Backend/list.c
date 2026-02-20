#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

void add_node_to_back(unsigned int field_order, list_t **list, void *data) {
    list_node_t *new_node = malloc(sizeof(list_node_t));
    new_node->next = new_node->prev = NULL;
    new_node->data = malloc((*list)->datasize);
    memcpy(new_node->data, data, (*list)->datasize);

    unsigned int offset, datasize;
    if (field_order == 1) {
        offset = (*list)->first_field_offset;
        datasize = (*list)->first_field_datasize;
    } else if (field_order == 2) {
        offset = (*list)->second_field_offset;
        datasize = (*list)->second_field_datasize;
    } else {
        offset = 0;
    }

    if (field_order) {
        *(char **)((char *)new_node->data + offset) = calloc(1, datasize);
        char *ptr1 = *(char **)((char *)new_node->data + offset);
        char *ptr2 = *(char **)((char *)data + offset);
    
        memcpy(ptr1, ptr2, datasize);
    }

    if ((*list)->head == NULL) {
        (*list)->head = new_node;
        (*list)->tail = new_node;

        return;
    } else {
        list_node_t *head = (*list)->head;

        while (head->next) {
            head = head->next;
        }

        head->next = new_node;
        new_node->prev = head;
        (*list)->tail = new_node;
    }
}

void add_node_to_front(unsigned int field_order, list_t **list, void *data) {
    list_node_t *new_node = malloc(sizeof(list_node_t));
    new_node->next = new_node->prev = NULL;
    new_node->data = malloc((*list)->datasize);
    memcpy(new_node->data, data, (*list)->datasize);

    unsigned int offset, datasize;
    if (field_order == 1) {
        offset = (*list)->first_field_offset;
        datasize = (*list)->first_field_datasize;
    } else if (field_order == 2) {
        offset = (*list)->second_field_offset;
        datasize = (*list)->second_field_datasize;
    } else {
        offset = 0;
    }

    if (field_order) {
        *(char **)((char *)new_node->data + offset) = calloc(1, datasize);
        strcpy(*(char **)(char *)new_node + offset, *(char **)((char *) data + offset));
    }

    if ((*list)->head == NULL) {
        // lista data este goala
        printf("lista este goala\n");
        (*list)->head = new_node;
        (*list)->tail = new_node;

        return;
    } else {
        new_node->next = (*list)->head;
        new_node->prev = NULL;
        (*list)->head->prev = new_node;
        (*list)->head = new_node;
    }
}

void create_list(list_t **list, unsigned int first_field_datasize, unsigned int second_field_datasize, unsigned int first_field_offset,
                                    unsigned int second_field_offset, unsigned int datasize, unsigned int keysize, void *key) {
    (*list) = malloc(sizeof(list_t));
    (*list)->keysize = keysize;
    (*list)->datasize = datasize;
    (*list)->head = (*list)->tail = NULL;

    if (keysize) {
        (*list)->key = calloc(keysize, 1);
    } else {
        (*list)->key = NULL;
    }

    (*list)->first_field_datasize = first_field_datasize;
    (*list)->second_field_datasize = second_field_datasize;
    (*list)->first_field_offset = first_field_offset;
    (*list)->second_field_offset = second_field_offset;
    (*list)->size = 0;

    if (key) {
        memcpy((*list)->key, key, (*list)->keysize);
    }
}

void free_list(unsigned int field_order, list_t **list) {
    list_node_t *head = (*list)->head;
    unsigned int offset;

    if (field_order == 1) {
        offset = (*list)->first_field_offset;
    } else if (field_order == 2) {
        offset = (*list)->second_field_offset; 
    } else {
        offset = 0;
    }

    while (head) {
        list_node_t *delete = head;
        head = head->next;

        if (field_order) {
            char *second_field_address = *(char **)((char *) delete->data + offset);
            free(second_field_address);
            second_field_address = NULL;
        }

        if (delete->data) {
            free(delete->data);
            delete->data = NULL;
        }

        free(delete);
        delete = NULL;
    }

    if ((*list)->key) {
        free((*list)->key);
        (*list)->key = NULL;
    }

    (*list)->head = (*list)->tail = NULL;
    
    free((*list));
    (*list) = NULL;
}

void remove_node_given_key(unsigned int field_order, void *key, list_t **list) {
    if (!(*list)) {
        return;
    }

    
    if (!(*list)->head) {
        return;
    }
    
    list_node_t *head = (*list)->head;
    unsigned int offset;
    
    if (field_order == 1) {
        offset = (*list)->first_field_offset;
    } else {
        offset = (*list)->second_field_offset;
    }

    while (head) {
        char *head_key = *(char **)((char*) head->data + offset);
        if (!strcmp(head_key, key)) {
            if (!head->prev && !head->next) {
                free(head_key);
                head_key = NULL;

                free((*list)->head->data);
                (*list)->head->data = NULL;
                free((*list)->head);
                (*list)->head = NULL;
                (*list)->tail = NULL;

                return;
            }

            list_node_t *delete = head;
            head = head->next;

            // if it is the first node, then we have to update the head
            if (!delete->prev) {
                (*list)->head = (*list)->head->next;
                (*list)->head->prev = NULL;
            } else {
                delete->prev->next = delete->next;
                if (delete->next) {
                    delete->next->prev = delete->prev;
                } else {
                    (*list)->tail = (*list)->tail->prev;
                    (*list)->tail->next = NULL;
                }
            }

            // if it is the last node, then we have to update the tail

            if (!delete->next) {
                delete->prev->next = NULL;
            }

            free(head_key);
            head_key = NULL;

            free(delete->data);
            delete->data = NULL;

            free(delete);
            delete = NULL;

            break;
        }

        head = head->next;
    }
}

void remove_node_from_front(unsigned int field_order, list_t **list) {
    if (!(*list)) {
        return;
    }

    
    if (!(*list)->head) {
        return;
    }
    
    unsigned int offset;
    
    if (field_order == 1) {
        offset = (*list)->first_field_offset;
    } else {
        offset = (*list)->second_field_offset;
    }

    if (!(*list)->head->next) {
        free(*(char **)((char *)(*list)->head->data + offset));
        *(char **)((char *)(*list)->head->data + offset) = NULL;

        free((*list)->head->data);
        (*list)->head->data = NULL;
        
        free((*list)->head);
        (*list)->head = NULL;
        
        (*list)->tail = NULL;
        return;
    }

    list_node_t *delete = (*list)->head;
    (*list)->head = (*list)->head->next;
    (*list)->head->prev = NULL;

    free(*(char **)((char *)delete->data + offset));
    *(char **)((char *)delete->data + offset) = NULL;

    free(delete->data);
    delete->data = NULL;
    
    free(delete);
    delete = NULL;
}

int get_item(unsigned int field_order, list_t **list, void *key) {
    if (!(*list)) {
        return 0;
    }

    
    if (!(*list)->head) {
        return 0;
    }
    
    list_node_t *head = (*list)->head;
    unsigned int offset;
    
    if (field_order == 1) {
        offset = (*list)->first_field_offset;
    } else {
        offset = (*list)->second_field_offset;
    }

    while (head) {
        char *head_key = *(char **)((char*) head->data + offset);
        if (!strcmp(head_key, key)) {
            return 1;            
        }

        head = head->next;
    }

    return 0;
}