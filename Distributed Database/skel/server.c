/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"
#include "utils.h"
#include "database.h"
#include "task_queue.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    // /* TODO */
    response *new_response = calloc(1, sizeof(response));
    
    new_response->server_id = s->server_id;
    new_response->server_log = calloc(MAX_LOG_LENGTH, 1);
    new_response->server_response = calloc(MAX_RESPONSE_LENGTH, 1);
    
    // we have to look for this document in cache first
    lru_cache *current_cache = s->cache;
    
    void *address = NULL;
    address = lru_cache_get(current_cache, (void *)doc_name);
    
    if (address == NULL) {
        // entry doesn't exist in cache, so we have to look for it in main database
        void *database_entry_address = database_get_entry(s->database, (void *)doc_name);
        
        if (!database_entry_address) {
            // entry doesn't exist in database, so we must create it here and in cache
            database_entry *new_database_entry = calloc(1, sizeof(database_entry));
            
            new_database_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
            new_database_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);
            
            strcpy(new_database_entry->document_content, doc_content);
            strcpy(new_database_entry->document_name, doc_name);
            
            unsigned int database_index = s->database->hash_function_docs(doc_name) % s->database->capacity;
            add_node(new_database_entry, &s->database->hashtable[database_index]->head, &s->database->hashtable[database_index]->tail, sizeof(database_entry));
            
            lru_cache_entry *new_entry = calloc(1, sizeof(lru_cache_entry));
            
            new_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);
            new_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
            
            strcpy(new_entry->document_content, doc_content);
            strcpy(new_entry->document_name, doc_name);
            
            if (!lru_cache_is_full(s->cache)) {
                
                // the cache is not full yet, so we can add this entry in it
                void *evicted = NULL;
                lru_cache_put(s->cache, new_entry->document_name, new_entry, &evicted);
                
                sprintf(new_response->server_log, LOG_MISS, doc_name);
                sprintf(new_response->server_response, MSG_C, doc_name);
            } else {
                // we need to find the evicted key here
                void *evicted_data = find_evicted_key(s->cache);
                void *evicted_key = evicted_data ? ((lru_cache_entry *) evicted_data)->document_name : NULL;
                
                sprintf(new_response->server_log, LOG_EVICT, doc_name,  (char *)evicted_key);
                sprintf(new_response->server_response, MSG_C, doc_name);

                lru_cache_put(s->cache, new_entry->document_name, new_entry, &evicted_key);
            }
            
            free(new_database_entry);
            free(new_entry);
        } else {
            // we have to modify the data in database
            database_entry *modified_entry = (database_entry *) database_entry_address;
            strcpy(modified_entry->document_content, doc_content);

            lru_cache_entry *new_entry = calloc(1, sizeof(lru_cache_entry));

            new_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);
            new_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);

            strcpy(new_entry->document_content, doc_content);
            strcpy(new_entry->document_name, doc_name);

            if (!lru_cache_is_full(s->cache)) {
                // the cache is not full yet, so we can add this entry in it
                void *evicted = NULL;

                sprintf(new_response->server_log, LOG_MISS, doc_name);
                sprintf(new_response->server_response, "%s", ((lru_cache_entry *)database_entry_address)->document_content);
               
                lru_cache_put(s->cache, new_entry->document_name, new_entry, &evicted);
            } else {
                // we need to find the evicted key here
                void *evicted_data = find_evicted_key(s->cache);
                void *evicted_key = evicted_data ? ((lru_cache_entry *) evicted_data)->document_name : NULL;

                sprintf(new_response->server_log, LOG_EVICT, doc_name,  (char *)evicted_key);
                sprintf(new_response->server_response, MSG_B, doc_name);

                lru_cache_put(s->cache, new_entry->document_name, new_entry, &evicted_key);
            }

            free(new_entry);
        }
    } else {
        // entry exists in cache, so we overwrite it
        void *database_entry_address = database_get_entry(s->database, (void *)doc_name);
        lru_cache_entry *modified_entry = (lru_cache_entry *)address;
        database_entry *modified_database_entry = (database_entry *)database_entry_address;
        strcpy(modified_entry->document_content, doc_content);

        if (database_entry_address) {
            strcpy(modified_database_entry->document_content, doc_content);
        }

        list_node_t *head = s->cache->lru_cache_list->head;
        void *retrieved_address = NULL;

        while (head) {
            if (!strcmp(((lru_cache_entry *)head->data)->document_name, doc_name)) {
                retrieved_address = ((lru_cache_entry *)head->data)->address;
                break;
            }
            head = head->next;
        }

        list_node_t *node_to_move = (list_node_t *) retrieved_address;

        if (node_to_move != NULL) {
            // If node is already the head, no need to move it.
            if (node_to_move->prev != NULL) {
                // Update the previous node's next pointer.
                node_to_move->prev->next = node_to_move->next;
                
                // If this node is not the tail, update the next node's previous pointer.
                if (node_to_move->next != NULL) {
                    node_to_move->next->prev = node_to_move->prev;
                } else {
                    // If node_to_move is the tail, update the tail pointer.
                    s->cache->lru_cache_list->tail = node_to_move->prev;
                }
        
                // Move the node to the front.
                node_to_move->prev = NULL;
                node_to_move->next = s->cache->lru_cache_list->head;
                
                // Update the current head's previous pointer, if the head exists.
                if (s->cache->lru_cache_list->head != NULL) {
                    s->cache->lru_cache_list->head->prev = node_to_move;
                }
                
                // Set the new head.
                s->cache->lru_cache_list->head = node_to_move;
            }
        }

        strcpy(((lru_cache_entry *)address)->document_content, doc_content);
        sprintf(new_response->server_log, LOG_HIT, doc_name);
        sprintf(new_response->server_response, MSG_B, doc_name);
    }

    return new_response;
}

static response
*server_get_document(server *s, char *doc_name) {
    /* TODO */
    response *new_response = calloc(1, sizeof(response));
    new_response->server_id = s->server_id;
    new_response->server_log = calloc(MAX_LOG_LENGTH, 1);
    new_response->server_response = calloc(MAX_RESPONSE_LENGTH, 1);
    
    // we have to look for this document in cache first

    lru_cache *current_cache = s->cache;
    void *address = lru_cache_get(current_cache, doc_name);
    
    if (address == NULL) {
        // entry doesn't exist in cache, so we have to look for it in main database
        void *database_entry_address = database_get_entry(s->database, (void *)doc_name);

        if (!database_entry_address) {
            // we don't have an entry for the specified document name, 

            sprintf(new_response->server_log, LOG_FAULT, doc_name);
            sprintf(new_response->server_response, "(null)");
        } else {
            // we have an entry in main memory, so we have to move it to cache

            lru_cache_entry *new_entry = calloc(1, sizeof(lru_cache_entry));

            new_entry->document_name = calloc(DOC_NAME_LENGTH + 1, 1);
            new_entry->document_content = calloc(DOC_CONTENT_LENGTH + 1, 1);

            strcpy(new_entry->document_content, ((database_entry *)database_entry_address)->document_content);
            strcpy(new_entry->document_name, ((database_entry *)database_entry_address)->document_name);

            if (!lru_cache_is_full(s->cache)) {
                // the cache is not full yet, so we can add this entry in it
                sprintf(new_response->server_log, LOG_MISS, doc_name);
                sprintf(new_response->server_response, "%s", ((lru_cache_entry *)database_entry_address)->document_content);

                lru_cache_put(s->cache, new_entry->document_name, new_entry, NULL);
            } else {
                // we need to find the evicted key here
                
                void *evicted_data = find_evicted_key(s->cache);
                void *evicted_key = evicted_data ? ((lru_cache_entry *) evicted_data)->document_name : NULL;

                sprintf(new_response->server_log, LOG_EVICT, doc_name, (char *)evicted_key);
                sprintf(new_response->server_response, "%s", ((lru_cache_entry *)database_entry_address)->document_content);

                lru_cache_put(s->cache, new_entry->document_name, new_entry, &evicted_key);
            }

            free(new_entry);
        }
    } else {
        sprintf(new_response->server_log, LOG_HIT, doc_name);
        sprintf(new_response->server_response, "%s", ((lru_cache_entry *)address)->document_content);

        // we have to update the list order so that this accessed cache memory should be in front

        list_node_t *head = s->cache->lru_cache_list->head;
        void *retrieved_address = NULL;

        while (head) {
            if (!strcmp(((lru_cache_entry *)head->data)->document_name, doc_name)) {
                retrieved_address = ((lru_cache_entry *)head->data)->address;
                break;
            }
            head = head->next;
        }

        list_node_t *node_to_move = (list_node_t *) retrieved_address;

        if (node_to_move != NULL) {
            // If node is already the head, no need to move it.
            if (node_to_move->prev != NULL) {
                // Update the previous node's next pointer.
                node_to_move->prev->next = node_to_move->next;
                
                // If this node is not the tail, update the next node's previous pointer.
                if (node_to_move->next != NULL) {
                    node_to_move->next->prev = node_to_move->prev;
                } else {
                    // If node_to_move is the tail, update the tail pointer.
                    s->cache->lru_cache_list->tail = node_to_move->prev;
                }
        
                // Move the node to the front.
                node_to_move->prev = NULL;
                node_to_move->next = s->cache->lru_cache_list->head;
                
                // Update the current head's previous pointer, if the head exists.
                if (s->cache->lru_cache_list->head != NULL) {
                    s->cache->lru_cache_list->head->prev = node_to_move;
                }
                
                // Set the new head.
                s->cache->lru_cache_list->head = node_to_move;
            }
        }
    }

    return new_response;
}

server *init_server(unsigned int cache_size, unsigned int server_id) {
    // /* TODO */
    server *new_server = calloc(1, sizeof(server));

    new_server->hash = calloc(DOC_NAME_LENGTH + 1, 1);
    
    new_server->server_id = server_id;
    new_server->cache = init_lru_cache(cache_size, DOC_NAME_LENGTH + 1, DOC_CONTENT_LENGTH + 1);
    new_server->task_queue = init_task_queue(sizeof(request));
    new_server->database = init_database(DOC_NAME_LENGTH + 1, MAXIMUM_DATABASE_ENTRIES, hash_string, hash_uint);
    new_server->get_req_received = 0;
    
    snprintf(new_server->hash, DOC_NAME_LENGTH + 1, "%u", new_server->cache->hash_function_servers(&server_id));

    return new_server;
}

response *server_handle_request(server *s, request *req) {
    /* TODO */
    response *new_response = NULL;

    // if the request type is get
    if (req->type == GET_DOCUMENT) {
        // if we still have elements in queue
        while (s->task_queue->tasks_list->tail != NULL) {
            // edit the requested document
            new_response = server_edit_document(s, ((request *)(s->task_queue->tasks_list->tail->data))->doc_name
            , ((request *)(s->task_queue->tasks_list->tail->data))->doc_content);
            
            // print the response
            PRINT_RESPONSE(new_response);
            
            // remove the extracted element from the queue
            free(((request *)(s->task_queue->tasks_list->tail->data))->doc_content);
            ((request *)(s->task_queue->tasks_list->tail->data))->doc_content = NULL;

            free(((request *)(s->task_queue->tasks_list->tail->data))->doc_name);
            ((request *)(s->task_queue->tasks_list->tail->data))->doc_name = NULL;
            
            remove_node(&s->task_queue->tasks_list->head, &s->task_queue->tasks_list->tail);
            s->task_queue->size--;
        } 

        s->task_queue->tasks_list->tail = NULL;
        s->task_queue->tasks_list->head = NULL;
        
        if (req->doc_name) {
            new_response = server_get_document(s, req->doc_name);
        }
    } else {
        new_response = malloc(sizeof(response));
        new_response->server_id = s->server_id;
        new_response->server_log = calloc(MAX_LOG_LENGTH + 1, 1);
        new_response->server_response = calloc(MAX_RESPONSE_LENGTH + 1, 1);

        request *request_copy = calloc(1, sizeof(request));

        request_copy->doc_content = calloc(DOC_CONTENT_LENGTH + 1, 1);
        request_copy->doc_name = calloc(DOC_NAME_LENGTH + 1, 1);
        strcpy(request_copy->doc_content, req->doc_content);
        strcpy(request_copy->doc_name, req->doc_name);

        // we have to insert the current request in queue
        add_task_in_queue(request_copy, &s->task_queue, sizeof(request));

        free(request_copy);
        sprintf(new_response->server_response, MSG_A, get_request_type_str(req->type), req->doc_name);
        sprintf(new_response->server_log, LOG_LAZY_EXEC, s->task_queue->size);
    }

    return new_response;
}

void free_server(server **s) {
    /* TODO */

    free((*s)->hash);
    (*s)->hash = NULL;
    
    free_database(&(*s)->database);
    free_lru_cache(&((*s)->cache));

    list_node_t *head = (*s)->task_queue->tasks_list->head;

    while (head) {
        list_node_t *curr = head;
        head = head->next;
        
        free(((request *)curr->data)->doc_content);
        ((request *)curr->data)->doc_content = NULL;
        free(((request *)curr->data)->doc_name);
        ((request *)curr->data)->doc_name = NULL;
        
        free(curr->data);
        curr->data = NULL;
    
        curr->next = curr->prev = NULL;
        free(curr);

        curr = NULL;
    }

    free((*s)->task_queue->tasks_list->data);
    (*s)->task_queue->tasks_list->data = NULL;

    free((*s)->task_queue->tasks_list);
    (*s)->task_queue->tasks_list = NULL;

    free((*s)->task_queue);
    (*s)->task_queue = NULL;

    free(*s);
    (*s) = NULL;
}
