#include "database.h"
#include "list.h"
#include <string.h>

database *init_database(unsigned int max_key_length, unsigned int max_database_capacity, unsigned int (*hash_string)(void *), unsigned int (*hash_uint)(void *)) {
    database *server_database = malloc(sizeof(database));

    server_database->capacity = max_database_capacity;
    server_database->size = 0;
    server_database->database_key_datasize = max_key_length;
    server_database->hashtable = malloc(sizeof(list_t *) * server_database->capacity);
    server_database->hash_function_docs = hash_string;
    server_database->hash_function_servers = hash_uint;
    
    for (int i = 0; i < server_database->capacity; i++) {
        create_list(sizeof(database_entry), &server_database->hashtable[i], max_key_length);
    }

    return server_database;
}

void *database_get_entry(database *database, void *key) {
    unsigned int index_to_check = database->hash_function_docs(key) % database->capacity;

    if (database->hashtable[index_to_check] == NULL) {
        return NULL;
    }

    list_t *list_to_check = database->hashtable[index_to_check];
    list_node_t *head = list_to_check->head;

    while (head != NULL) {
        if (!strncmp((char *)key, ((database_entry *)(head->data))->document_name, database->database_key_datasize)) {
            return head->data;
        }
        
        head = head->next;
    }

    return NULL;
}

void free_database(database **database) {
    (*database)->hash_function_docs = NULL;
    (*database)->hash_function_servers = NULL;

    for (int i = 0; i < (*database)->capacity; i++) {
        list_node_t *head = (*database)->hashtable[i]->head;

        while (head) {
            free(((database_entry *)head->data)->document_name);
            ((database_entry *)head->data)->document_name = NULL;
            free(((database_entry *)head->data)->document_content);
            ((database_entry *)head->data)->document_content = NULL;

            head = head->next;
        }
        free_list_normally(&(*database)->hashtable[i]->head);

        free((*database)->hashtable[i]->data);
        (*database)->hashtable[i]->data = NULL;

        free((*database)->hashtable[i]);
        (*database)->hashtable[i] = NULL;
    }

    free ((*database)->hashtable);
    (*database)->hashtable = NULL;

    free((*database));
    (*database) = NULL;
}