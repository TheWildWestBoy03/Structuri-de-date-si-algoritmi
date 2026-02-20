/*
 * Copyright (c) 2024, <>
 */

#include "load_balancer.h"
#include "server.h"
#include <stdlib.h>

load_balancer *init_load_balancer(bool enable_vnodes) {
	/* TODO */
    load_balancer *new_load_balancer = calloc(1, sizeof(load_balancer));
    new_load_balancer->enable_vnodes = enable_vnodes;
    new_load_balancer->hash_function_docs = hash_string;
    new_load_balancer->hash_function_servers = hash_uint;
    new_load_balancer->number_of_servers = 0;
    new_load_balancer->number_of_transitioning_documents = 0;
    new_load_balancer->servers = calloc(MAX_SERVERS, sizeof(server *));

    create_list(sizeof(server_resume), &new_load_balancer->server_tags, sizeof(int));

    for (int i = 0; i < MAX_SERVERS; i++) {
        new_load_balancer->servers[i] = NULL;
    }

    new_load_balancer->entries_ready_for_moving = calloc(MAXIMUM_DATABASE_ENTRIES, sizeof(database_entry *));

    for (int i = 0; i < MAXIMUM_DATABASE_ENTRIES; i++) {
        new_load_balancer->entries_ready_for_moving[i] = calloc(1, sizeof(database_entry));
        new_load_balancer->entries_ready_for_moving[i]->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
        new_load_balancer->entries_ready_for_moving[i]->document_name = calloc(DOC_NAME_LENGTH + 1, 1);
    }

    return new_load_balancer;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    /* TODO: Remove test_server after checking the server implementation */
    main->servers[server_id] = init_server(cache_size, server_id);
    main->number_of_servers += 1;

    server_resume *server_entry = calloc(1, sizeof(*server_entry));
    server_entry->server_id = server_id;
    server_entry->hash = calloc(DOC_NAME_LENGTH + 1, 1);
    strncpy(server_entry->hash, main->servers[server_id]->hash, DOC_NAME_LENGTH);
    server_entry->hash[DOC_NAME_LENGTH] = '\0';
    
    // printf("-------ADD SERVER-----------\n");
    // printf("acum adaug serverul %ld cu hashul %s\n", server_id, server_entry->hash);
    add_node_in_ascending_order(&(main->server_tags), server_entry, 8);
    free(server_entry);
    server_entry = NULL;

    if (main->number_of_servers == 1) {
        return;
    }
    // print_server_tags(main->server_tags->head);
    
    // declaration of source and destination server hashes
    char *source_hash = calloc(DOC_NAME_LENGTH, 1);
    char *destination_hash = calloc(DOC_NAME_LENGTH, 1);
    char *key_hash = calloc(DOC_NAME_LENGTH, 1);
    
    strcpy(destination_hash, main->servers[server_id]->hash);
    int source_server_id = 0, destination_server_id = server_id;
    
    /* compute the server which is the next in the hash ring */
    list_node_t *head_copy = main->server_tags->head;
    server_entry = NULL;
    
    while (head_copy) {
        server_entry = head_copy->data;
        if (server_entry->server_id == server_id) {
            break;
        }
        
        head_copy = head_copy->next;
    }
    
    if (!head_copy) {
        source_server_id = ((server_resume *)(main->server_tags->head)->data)->server_id;
    } else {
        if (head_copy->next) {
            source_server_id = ((server_resume *)head_copy->next->data)->server_id;
        } else {
            source_server_id = ((server_resume *)(main->server_tags->head)->data)->server_id;
        }
    }
    
    // printf("vecinu este %d\n", source_server_id);
    // printf("de aici trebuie sa luam documentele pentru noul server %u\n", source_server_id);
    // printf("-------ADD SERVER-----------\n");

    if (!main->servers[source_server_id]->database) {
        free(source_hash);
        free(destination_hash);
        free(key_hash);

        return;
    }

    request *new_req = calloc(1, sizeof(request));
    new_req->type = GET_DOCUMENT;
    
    new_req->doc_name = NULL;
    new_req->doc_content = NULL;
    
    response *resp = server_handle_request(main->servers[source_server_id], new_req);

    free(new_req);

    if (!main->servers[destination_server_id]) {
        free(source_hash);
        free(destination_hash);
        free(key_hash);
        
        return;
    }

    list_t **current_database = main->servers[source_server_id]->database->hashtable;
    
    // looking for documents to be moved from the source server
    for (int j = 0; j < MAXIMUM_DATABASE_ENTRIES; j++) {
        list_node_t *current_database_entry_head = current_database[j]->head;
        
        while (current_database_entry_head) {
            list_node_t *current_db_node = current_database_entry_head;
            database_entry *current_db_entry = (database_entry *)(current_db_node->data);

            char *key = current_db_entry->document_name;

            snprintf(key_hash, DOC_NAME_LENGTH, "%u", main->hash_function_docs(key));
            
            // printf("Current database docname is: %s while hash is: %s\n", current_db_entry->document_name, key_hash);
            unsigned int destination_number = atoi(destination_hash);
            unsigned int key_hash_number = atoi(key_hash);
            // printf("destination_number=%u and key_hash_number=%u\n", destination_number, key_hash_number);
            
            // here i am copying the current database entry into the collection of documents ready for moving
            if (key_hash_number <= destination_number) {
                // printf("se adauga\n");
                strcpy(((database_entry *)main->entries_ready_for_moving[main->number_of_transitioning_documents])->document_content, current_db_entry->document_content);
                strcpy(((database_entry *)main->entries_ready_for_moving[main->number_of_transitioning_documents])->document_name, current_db_entry->document_name);

                main->number_of_transitioning_documents++;
                main->servers[source_server_id]->cache->size--;
                
                if (main->servers[source_server_id]->cache->lru_cache_list) {
                    list_node_t **head_ref = &main->servers[source_server_id]->cache->lru_cache_list->head;
                    list_node_t **tail_ref = &main->servers[source_server_id]->cache->lru_cache_list->tail;
                    list_node_t *current = *head_ref;

                    while (current) {
                        if (!strcmp(((lru_cache_entry *)current->data)->document_name, (char *)key)) {
                            // Free allocated memory for document
                            free(((lru_cache_entry *)current->data)->document_name);
                            ((lru_cache_entry *)current->data)->document_name = NULL;
                            free(((lru_cache_entry *)current->data)->document_content);
                            ((lru_cache_entry *)current->data)->document_content = NULL;

                            free(current->data);
                            current->data = NULL;

                            // Update list pointers
                            if (current->prev) {
                                current->prev->next = current->next;
                            } else {
                                // If removing the head, update head pointer
                                *head_ref = current->next;
                            }

                            if (current->next) {
                                current->next->prev = current->prev;
                            } else {
                                // If removing the tail, update tail pointer
                                *tail_ref = current->prev;
                            }

                            list_node_t *temp = current;
                            current = current->next;
                            free(temp);
                            break;
                        } else {
                            current = current->next;
                        }
                    }
    
                    lru_cache_remove(main->servers[source_server_id]->cache, key);
                }
            }
            
            current_database_entry_head = current_database_entry_head->next;
        }
    }
    
    // move the correct documents to the new destination server
    server *destination_server = main->servers[destination_server_id];
    lru_cache *server_cache = destination_server->cache;
    
    for (int j = 0; j < main->number_of_transitioning_documents; j++) {
        database_entry *current_entry = main->entries_ready_for_moving[j];
        // printf("docname is: %s and its hash is: %u \n", current_entry->document_name, main->hash_function_docs(current_entry->document_name));
        unsigned int current_entry_index = main->hash_function_docs(current_entry->document_name) % destination_server->database->capacity;
        
        database_entry *new_entry = calloc(1, sizeof(database_entry));
        new_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
        new_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);

        strcpy(new_entry->document_content, current_entry->document_content);
        strcpy(new_entry->document_name, current_entry->document_name);
        
        add_node(new_entry, &destination_server->database->hashtable[current_entry_index]->head, &destination_server->database->hashtable[current_entry_index]->tail, sizeof(database_entry));
        
        free(new_entry);
        new_entry = NULL;
        
        current_entry->document_content[0] = 0;
        current_entry->document_name[0] = 0;
    }
    
    //reset the number of documents stored for forwarding
    main->number_of_transitioning_documents = 0;

    free(source_hash);
    free(destination_hash);
    free(key_hash);
}

void loader_remove_server(load_balancer* main, int server_id) {
    /* TODO */

    // compute the server which is the suitable neighbor for the server

    main->number_of_servers--;
    if (!main->servers[server_id] || !main->number_of_servers) {
        return;
    }

    int neighbor = 0;
    request *new_request = malloc(sizeof(request));
    
    new_request->type = GET_DOCUMENT;
    new_request->doc_content = NULL;
    new_request->doc_name = NULL;

    response *response = server_handle_request(main->servers[server_id], new_request);

    free(new_request);
    new_request = NULL;

    remove_node_with_key(&main->server_tags, &server_id, 8, sizeof(unsigned int));

    // printf("-------REMOVE SERVER-----------\n");
    // printf("acum sterg serverul %ld cu hashul %u\n", server_id, main->hash_function_servers(&server_id));
    // print_server_tags(main->server_tags->head);
    // printf("-------REMOVE SERVER-----------\n");

    list_node_t *head = main->server_tags->head;

    unsigned int deleted_server_hash = main->hash_function_servers(&server_id);

    while (head) {
        unsigned int key1 = atoi(((server_resume *) (head->data))->hash);
        unsigned int curr_server_id = ((server_resume *)head->data)->server_id;
    
        if (key1 > deleted_server_hash) {
            break;
        } else {
            if (key1 == deleted_server_hash) {
                if (((server_resume *)head->data)->server_id > server_id) {
                    break;
                }
            }
        }

        head = head->next;
    }

    if (!head) {
        neighbor = ((server_resume *)(main->server_tags->head)->data)->server_id;
    } else {
        neighbor = ((server_resume *) head->data)->server_id;
    }

    // printf("noul vecin este %d\n", neighbor);
    database *current_database = main->servers[neighbor]->database;
    database *removed_db = main->servers[server_id]->database;

    for (int i = 0; i < MAXIMUM_DATABASE_ENTRIES; i++) {
        list_t *list = removed_db->hashtable[i];
        if (!list) {
            continue;
        }
        list_node_t *head = list->head;

        while (head) {
            database_entry *db_entry = (database_entry *)head->data;

            unsigned int index_to_lookup = main->hash_function_docs(db_entry->document_name) % current_database->capacity;
            database_entry *new_entry = calloc(1, sizeof(database_entry));
            new_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
            new_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);

            strcpy(new_entry->document_name, db_entry->document_name);
            strcpy(new_entry->document_content, db_entry->document_content);

            add_node(new_entry, &(current_database->hashtable[index_to_lookup]->head),
                        &(current_database->hashtable[index_to_lookup]->tail), sizeof(database_entry)); 

            free(new_entry);
            new_entry = NULL;

            head = head->next;
        }
    }

    if (main->servers[server_id]) {
        free_server(&(main->servers[server_id]));
        main->servers[server_id] = NULL;
    }
}

response *loader_forward_request(load_balancer* main, request *req) {
    /* TODO */
    // printf("------LOADER FORWARD REQUEST--------\n");
    if (main->number_of_servers == 1) {
        list_node_t *head = main->server_tags->head;
        unsigned int destination_server = ((server_resume *)head->data)->server_id;

        return server_handle_request(main->servers[destination_server], req);
    }

    char *document_hash = calloc(DOC_NAME_LENGTH + 1, 1);
    snprintf(document_hash, DOC_NAME_LENGTH , "%u", main->hash_function_docs(req->doc_name));
    
    int destination_server = 0;
    // printf("%s %s\n", req->doc_name, document_hash);
    list_node_t *head = main->server_tags->head;
    unsigned int offset = sizeof(unsigned int) * 2;

    // print_server_tags(main->server_tags->head);
    while (head) {
        char *key1 = *(char **)((char *)head->data + offset);
        unsigned int numberized_key1 = atoi(key1);
        unsigned int numberized_key2 = atoi(document_hash);

        if (numberized_key1 >= numberized_key2) {
            break;
        }

        head = head->next;
    }

    if (!head) {
        destination_server = *(unsigned int *) main->server_tags->head->data;
    } else {
        destination_server = *(unsigned int*) (head->data);
    }

    // printf("next server should be... %d\n", destination_server);
    free(document_hash);
    document_hash = NULL;

    // printf("------LOADER FORWARD REQUEST--------\n");
    
    return server_handle_request(main->servers[destination_server], req);
}

void free_load_balancer(load_balancer** main) {
    /* TODO: get rid of test_server after testing the server implementation */

    if ((*main)->server_tags) {
        list_node_t *head = (*main)->server_tags->head;
    
        while (head) {
            list_node_t *delete = head;
    
            head = head->next;
    
            free(((server_resume *)(delete->data))->hash);
            ((server_resume *)(delete->data))->hash = NULL;

            free(delete->data);
            delete->data = NULL;
    
            free(delete);
            delete = NULL;
        }
    
        free((*main)->server_tags->data);
        (*main)->server_tags->data = NULL;
    
        free((*main)->server_tags);
        (*main)->server_tags = NULL;
    }

    for (int i = 0; i < MAX_SERVERS; i++) {
        if ((*main)->servers[i]) {
            free_server(&(*main)->servers[i]);
            (*main)->servers[i] = NULL;
        }
    }

    free((*main)->servers);
    (*main)->servers = NULL;

    for (int i = 0; i < MAXIMUM_DATABASE_ENTRIES; i++) {
        free((*main)->entries_ready_for_moving[i]->document_content);
        free((*main)->entries_ready_for_moving[i]->document_name);

        (*main)->entries_ready_for_moving[i]->document_name = NULL;
        (*main)->entries_ready_for_moving[i]->document_content = NULL;

        free((*main)->entries_ready_for_moving[i]);
        (*main)->entries_ready_for_moving[i] = NULL;
    }

    free((*main)->entries_ready_for_moving);
    (*main)->entries_ready_for_moving = NULL;

    free(*main);
    *main = NULL;
}

void print_server_tags(list_node_t *head) {
    while (head) {
        printf("Server id is: %d\n", (((server_resume *)head->data)->server_id));
        printf("Server hash is: %s\n", (((server_resume *)head->data)->hash));

        head = head->next;
    }
}