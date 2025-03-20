#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// free memory helper structures

typedef struct SegregatedListNode {
    struct SegregatedListNode *next, *prev;
    void *data;  
} SegregatedListNode;

typedef struct SegregatedFreeList {
    SegregatedListNode *head;
    void *data;
} SegregatedFreeList;

typedef struct HeapStructure {
    SegregatedFreeList **segregated_free_lists;
    int heap_start_address, number_of_lists, data_size;
    int total_memory, total_free_memory, maximum_size;
    int total_blocks;
    int capacity;
    int tip_reconstituire;
    int free_calls;
} HeapStructure;

typedef struct ListData {
    int size;
    int start_address;
    int number_of_blocks;
    int data_size;
} ListData;

typedef struct BlockData {
    int size;
    int start_address;
    int used;
} BlockData;

/*
    this is a structure which represents the list of allocated zones, locating in memory.
    Every node has a brief description, including the start_address, the size, the payload and some permissions.
    The logistics of this data structures make mandatory the usage of *prev and *next nodes;
*/
typedef struct AllocatedNode {
    int start_address;
    int size;
    char *payload;
    int permissions;
    struct AllocatedNode *next, *prev;
} AllocatedNode;

typedef struct HeapArena {
    int total_allocated_memory;
    int total_mallocs;
    int malloc_calls;
    int total_fragmentations;
    AllocatedNode *head;
} HeapArena;

void get_initialization_heap_parameters(char *line, int *start_address, int *number_of_lists, int *maximum_size, int *tip_reconstituire) {
    char *word = strtok(line, " ");
    word = strtok(NULL, " ");
       
    *start_address = (int) strtol(word, NULL, 0);
    word = strtok(NULL, " ");

    *number_of_lists = atoi(word);
    word = strtok(NULL, " ");

    *maximum_size = atoi(word);
    word = strtok(NULL, " ");

    *tip_reconstituire = atoi(word);
    word = strtok(NULL, " ");
}


void sort_the_segregated_free_lists(HeapStructure **main_heap) {
    SegregatedFreeList *temp;

    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        for (int j = i + 1; j < (*main_heap)->number_of_lists; j++) {
            if (((ListData *)(*main_heap)->segregated_free_lists[i]->data)->size > 
                ((ListData *)(*main_heap)->segregated_free_lists[j]->data)->size) {
                
                temp = (*main_heap)->segregated_free_lists[i];
                (*main_heap)->segregated_free_lists[i] = (*main_heap)->segregated_free_lists[j];
                (*main_heap)->segregated_free_lists[j] = temp;
            }
        }
    }
}

void add_info_block_to_segregated_list(SegregatedFreeList **list, BlockData *info_block) {
    SegregatedListNode *head = (*list)->head;
    SegregatedListNode *prev = NULL;

    int datasize = ((ListData *)((*list)->data))->data_size;

    SegregatedListNode *new_node = (SegregatedListNode *) malloc(sizeof(SegregatedListNode));
    new_node->data = malloc(datasize);
    memcpy((BlockData *)new_node->data, info_block, datasize);

    new_node->next = NULL;
    new_node->prev = NULL;
    
    if (head == NULL) {
        (*list)->head = new_node;
        return;
    } else {
        prev = head;
        while (head != NULL && ((BlockData *)(head->data))->start_address < info_block->start_address) {
            prev = head;
            head = head->next;
        }

        if (head != NULL) {
            if (head == (*list)->head) {
                new_node->next = head;
                head->prev = new_node;
                (*list)->head = new_node;
            } else {
                new_node->next = head;
                new_node->prev = head->prev;
                head->prev->next = new_node;
                head->prev = new_node;
            }
        } else {
            new_node->prev = prev;
            prev->next = new_node;
        }
    }
}

void add_info_block_to_arena(AllocatedNode **main_head, AllocatedNode *new_node) {
    AllocatedNode *prev = *main_head, *head = *main_head;
    if (!(head)) {
        (*main_head) = new_node;
        return;
    }

    while ((head) != NULL && (head)->start_address < new_node->start_address) {
        prev = (head);
        (head) = (head)->next;
    }

    if ((head) != NULL) {
        if ((head) == (*main_head)) {
            new_node->next = (head);
            (head)->prev = new_node;
            (*main_head) = new_node;
        } else {
            new_node->next = (head);
            new_node->prev = (head)->prev;
            (head)->prev->next = new_node;
            (head)->prev = new_node;
        }
    } else {
        prev->next = new_node;
        new_node->prev = prev;
    }

    return;
}

void create_heap(HeapArena **heap_arena, HeapStructure **main_heap, char *line) {
    int start_address, number_of_lists, maximum_size, tip_reconstituire;
    get_initialization_heap_parameters(line, &start_address, &number_of_lists, &maximum_size, &tip_reconstituire);

    // free memory heap structure initialization
    (*main_heap) = (HeapStructure *) malloc(sizeof(**main_heap));
    (*main_heap)->data_size = sizeof(SegregatedFreeList);
    (*main_heap)->heap_start_address = start_address;
    (*main_heap)->number_of_lists = number_of_lists;
    (*main_heap)->tip_reconstituire = tip_reconstituire;
    (*main_heap)->maximum_size = maximum_size;
    (*main_heap)->segregated_free_lists = (SegregatedFreeList **) malloc(sizeof(SegregatedFreeList *) * 1000);
    (*main_heap)->total_memory = (*main_heap)->number_of_lists * (*main_heap)->maximum_size;
    (*main_heap)->total_free_memory = (*main_heap)->total_memory;
    (*main_heap)->free_calls = 0;
    (*main_heap)->total_blocks = 0;
    (*main_heap)->capacity = (*main_heap)->number_of_lists;

    if ((*main_heap)->segregated_free_lists == NULL) {
        exit(1);
    }

    // allocated zones structure initialization

    (*heap_arena) = (HeapArena *) malloc(sizeof(HeapArena));
    (*heap_arena)->head = NULL;
    (*heap_arena)->total_allocated_memory = 0;
    (*heap_arena)->total_fragmentations = 0;
    (*heap_arena)->total_mallocs = 0;
    (*heap_arena)->malloc_calls = 0;

    int first_size = 8;
    int first_address = start_address;

    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        ListData *listData = (ListData *) malloc(sizeof(ListData));
        (*main_heap)->segregated_free_lists[i] = (SegregatedFreeList *) malloc((*main_heap)->data_size);
        (*main_heap)->segregated_free_lists[i]->head = NULL;

        listData->size = first_size;
        listData->start_address = first_address;
        listData->data_size = sizeof(BlockData);
        listData->number_of_blocks = (*main_heap)->maximum_size / listData->size;

        first_size *= 2;
        first_address += maximum_size;

        (*main_heap)->segregated_free_lists[i]->data = malloc(sizeof(ListData));
        memcpy((*main_heap)->segregated_free_lists[i]->data, listData, sizeof(ListData));

        free(listData);
    }

    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        int number_of_addings = ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->number_of_blocks;
        int start_address = ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->start_address;
        int node_size = ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->size;
        (*main_heap)->total_blocks += number_of_addings;

        while(number_of_addings) {
            BlockData *new_block_info = malloc(sizeof(BlockData));
            new_block_info->size = node_size;
            new_block_info->start_address = start_address;
            new_block_info->used = 0;

            add_info_block_to_segregated_list(&(*main_heap)->segregated_free_lists[i], new_block_info);
            start_address += node_size;
            number_of_addings -= 1;
            
            free(new_block_info);
        }
    }
}

// MALLOC FUNCTIONS

void get_malloc_parameters(char *line, int *block_size) {
    char *word = strtok(line, " ");
    word = strtok(NULL, " ");

    *block_size = atoi(word);
}

void handle_malloc(HeapArena **heap_arena, HeapStructure **main_heap ,char *line) {
    int block_size = 0;
    get_malloc_parameters(line, &block_size);

    int malloc_done = 0;
    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        SegregatedListNode *head = (*main_heap)->segregated_free_lists[i]->head;

        while (head != NULL) {
            if (((BlockData *)(head->data))->size >= block_size) {
                malloc_done = 1;
                // creating a new allocated node and deleting the node from the segregated free list
                AllocatedNode *new_allocated_node = (AllocatedNode *) malloc(sizeof(AllocatedNode));
                new_allocated_node->next = new_allocated_node->prev = new_allocated_node->payload = NULL;
                new_allocated_node->permissions = 777;
                new_allocated_node->size = block_size;
                new_allocated_node->start_address = ((BlockData *)(head->data))->start_address;
                new_allocated_node->payload = malloc(new_allocated_node->size + 1);
                add_info_block_to_arena(&(*heap_arena)->head, new_allocated_node);

                int saved_address = ((BlockData *)(head->data))->start_address + block_size;
                int saved_size = ((BlockData *)(head->data))->size;

                // eliminating the current node from the free memory structure
                if (((ListData *)(*main_heap)->segregated_free_lists[i]->data)->number_of_blocks == 1) {
                    free(head->data);
                    head->data = NULL;
                    free(head);
                    head = NULL;
                    (*main_heap)->segregated_free_lists[i]->head = NULL;
                } else if (head -> next != NULL && head != (*main_heap)->segregated_free_lists[i]->head) {
                    SegregatedListNode *deleted_node = head;
                    head->prev->next = head->next;
                    head->next->prev = head->prev;
    
                    free(deleted_node->data);
                    deleted_node->data = NULL;
                    free(deleted_node);
                    deleted_node = NULL;
                } else if (head -> next == NULL) {
                    SegregatedListNode *deleted_node = head;
                    head->prev->next = NULL;
                    free(deleted_node->data);
                    deleted_node->data = NULL;
                    free(deleted_node);
                    deleted_node = NULL;
                } else if (head == (*main_heap)->segregated_free_lists[i]->head) {
                    SegregatedListNode *deleted_node = head;
                    head = head->next;
                    head->prev = NULL;
                    (*main_heap)->segregated_free_lists[i]->head = head;

                    free(deleted_node->data);
                    deleted_node->data = NULL;
                    free(deleted_node);
                    deleted_node = NULL;
                }

                // modifying the statements about total allocations so far
                (*main_heap)->total_free_memory -= block_size;
                (*main_heap)->total_blocks--;
                (*heap_arena)->total_allocated_memory += block_size;
                (*heap_arena)->total_mallocs += 1;
                (*heap_arena)->malloc_calls += 1;

                ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->number_of_blocks -= 1;

                // in this case we want to treat the fragmentation case
                if (saved_size > block_size) {
                    BlockData *new_block = calloc(1, sizeof(BlockData));
                    new_block->size = saved_size - block_size;
                    new_block->start_address = saved_address;
                    new_block->used = 0;
                    
                    ((*heap_arena)->total_fragmentations) += 1;
                    (*main_heap)->total_blocks++;

                    int list_found = 0;
    
                    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
                        if (new_block->size == ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->size) {
                            list_found = 1;
                            ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->number_of_blocks += 1;
                            add_info_block_to_segregated_list(&(*main_heap)->segregated_free_lists[i], new_block);
                            free(new_block);
                            return;
                        }
                    }

                    // adaug blocul liber de memorie desprins din blocul de memorie mare si fragmentat
                    if (!list_found) {
                        if ((*main_heap)->capacity == (*main_heap)->number_of_lists) {
                            (*main_heap)->segregated_free_lists = realloc((*main_heap)->segregated_free_lists, (*main_heap)->capacity * 2 * sizeof(*(*main_heap)->segregated_free_lists));
                            (*main_heap)->capacity *= 2;

                            if (!(*main_heap)->segregated_free_lists) {
                                exit(1);
                            }
                        }

                        int current_front_index = (*main_heap)->number_of_lists;
                        (*main_heap)->segregated_free_lists[current_front_index] = malloc(sizeof(SegregatedFreeList));
                        (*main_heap)->segregated_free_lists[current_front_index]->head = NULL;
                        (*main_heap)->segregated_free_lists[current_front_index]->data = malloc(sizeof(ListData));

                        ((ListData *)(*main_heap)->segregated_free_lists[current_front_index]->data)->data_size = sizeof(BlockData);
                        ((ListData *)(*main_heap)->segregated_free_lists[current_front_index]->data)->start_address = new_block->start_address;
                        ((ListData *)(*main_heap)->segregated_free_lists[current_front_index]->data)->number_of_blocks = 1;
                        ((ListData *)(*main_heap)->segregated_free_lists[current_front_index]->data)->size = new_block->size;
                        
                        add_info_block_to_segregated_list(&(*main_heap)->segregated_free_lists[current_front_index], new_block);
                        (*main_heap)->number_of_lists = current_front_index + 1;
                    }
                    free(new_block);

                    sort_the_segregated_free_lists(main_heap);
                }

                return;
            }

            head = head->next;
        }
    }

    if (!malloc_done) {
        printf("Out of memory\n");
    }
}

// DUMP MEMORY LOGIC SECTION

void dump_memory(HeapArena **heap_arena, HeapStructure **main_heap) {
    printf("+++++DUMP+++++\n");

    printf("Total memory: %d bytes\n", (*main_heap)->total_memory);
    printf("Total allocated memory: %d bytes\n", (*main_heap)->total_memory - (*main_heap)->total_free_memory);
    printf("Total free memory: %d bytes\n", (*main_heap)->total_free_memory);
    
    printf("Free blocks: %d\n", (*main_heap)->total_blocks);
    printf("Number of allocated blocks: %d\n", (*heap_arena)->total_mallocs);
    printf("Number of malloc calls: %d\n", (*heap_arena)->malloc_calls);
    printf("Number of fragmentations: %d\n", (*heap_arena)->total_fragmentations);

    printf("Number of free calls: %d\n", (*main_heap)->free_calls);

    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        if (((ListData*) ((*main_heap)->segregated_free_lists[i]->data))->number_of_blocks) {
            printf("Blocks with %d bytes - %d free block(s) : ", ((ListData*) ((*main_heap)->segregated_free_lists[i]->data))->size, ((ListData*) ((*main_heap)->segregated_free_lists[i]->data))->number_of_blocks);
            
            SegregatedListNode *head = (*main_heap)->segregated_free_lists[i]->head;
            while (head) {
                if (!head->next) {
                    printf("0x%x", ((BlockData *)head->data)->start_address);
                } else {
                    printf("0x%x ", ((BlockData *)head->data)->start_address);
                }
                head = head -> next;
            }
    
            printf("\n");
        }
    }

    AllocatedNode *allocated_nodes_head = (*heap_arena)->head;
    printf("Allocated blocks :");

    while (allocated_nodes_head) {
        if (allocated_nodes_head == (*heap_arena)->head) {
            printf(" ");
        }
        if (allocated_nodes_head -> next) {
            printf("(0x%x - %d) ", allocated_nodes_head->start_address, allocated_nodes_head->size);
        } else {
            printf("(0x%x - %d)", allocated_nodes_head->start_address, allocated_nodes_head->size);
        }

        allocated_nodes_head = allocated_nodes_head->next;
    }
    printf("\n");
    printf("-----DUMP-----\n");
}

// FREE HANDLING LOGIC SECTION

void get_free_parameters(char *line, int *address) {
    char *word = strtok(line, " ");
    word = strtok(NULL, " ");

    *address = strtol(word, NULL, 0);
}

void handle_free(HeapArena **arena_heap, HeapStructure **main_heap, char *line) {
    int requested_address = 0;

    get_free_parameters(line, &requested_address);

    // look up in the structure choosen for handling the allocated memory
    AllocatedNode *allocated_nodes_head = (*arena_heap)->head;
    int node_found_to_free = 0;
    int new_list_necessary = 1;

    while (allocated_nodes_head) {
        if (allocated_nodes_head->start_address == requested_address) {
            node_found_to_free = 1;
            AllocatedNode *t = allocated_nodes_head;
            
            // adding the freed block to the free zones list
            BlockData *new_block_info = malloc(sizeof(BlockData));
            new_block_info->size = t->size;
            new_block_info->used = 0;
            new_block_info->start_address = t->start_address;

            // modify the main structures properly
            (*arena_heap)->total_allocated_memory -= t->size;
            (*main_heap)->free_calls += 1;
            (*main_heap)->total_free_memory += t->size;
            (*main_heap)->total_blocks += 1;

            for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
                if (((ListData *)(*main_heap)->segregated_free_lists[i]->data)->size == t->size) {
                    ((ListData *)(*main_heap)->segregated_free_lists[i]->data)->number_of_blocks += 1;
                    new_list_necessary = 0;
                    add_info_block_to_segregated_list(&(*main_heap)->segregated_free_lists[i], new_block_info);
                }
            }

            // we need a new list for this zone side
            if (new_list_necessary) {
                if ((*main_heap)->capacity == (*main_heap)->number_of_lists) {
                    (*main_heap)->segregated_free_lists = realloc(
                        (*main_heap)->segregated_free_lists, 
                        (*main_heap)->capacity * 2 * sizeof(*(*main_heap)->segregated_free_lists)
                    );

                    (*main_heap)->capacity *= 2;
                    if (!(*main_heap)->segregated_free_lists) {
                        exit(1);
                    }
                }
            
                int current_front_index = (*main_heap)->number_of_lists;
                
                (*main_heap)->segregated_free_lists[current_front_index] = malloc(sizeof(SegregatedFreeList));
                (*main_heap)->segregated_free_lists[current_front_index]->head = NULL;
                ((*main_heap)->segregated_free_lists[current_front_index]->data) = malloc(sizeof(ListData));

                ((ListData *)((*main_heap)->segregated_free_lists[current_front_index]->data))->size = t->size;
                ((ListData *)((*main_heap)->segregated_free_lists[current_front_index]->data))->number_of_blocks = 1;
                ((ListData *)((*main_heap)->segregated_free_lists[current_front_index]->data))->start_address = t->start_address;
                ((ListData *)((*main_heap)->segregated_free_lists[current_front_index]->data))->data_size = sizeof(BlockData);

                add_info_block_to_segregated_list(&(*main_heap)->segregated_free_lists[current_front_index], new_block_info);

                (*main_heap)->number_of_lists = current_front_index + 1;
            }
            
            free(new_block_info);
            sort_the_segregated_free_lists(main_heap);
            if ((*arena_heap)->total_mallocs == 1) {
                free(t->payload);
                t->payload = NULL;
                free(t);
                (*arena_heap)->head = NULL;
            } else if (allocated_nodes_head != (*arena_heap)->head && allocated_nodes_head->next != NULL) {
                allocated_nodes_head->prev->next = allocated_nodes_head->next;
                allocated_nodes_head->next->prev = allocated_nodes_head->prev;
                free(t->payload);
                t->payload = NULL;
                free(t);
            } else if (allocated_nodes_head == (*arena_heap)->head){
                allocated_nodes_head->next->prev = NULL;
                (*arena_heap)->head = allocated_nodes_head->next;
                free(t->payload);
                t->payload = NULL;
                free(t);
            } else if (allocated_nodes_head->next == NULL) {
                allocated_nodes_head->prev->next = NULL;
                free(t->payload);
                t->payload = NULL;
                free(t);
            }

            t = NULL;
            (*arena_heap)->total_mallocs -= 1;
            return;
        }

        allocated_nodes_head = allocated_nodes_head->next;
    }

    if (!node_found_to_free) {
        printf("Invalid free\n");
    }
}

void delete_heap_arena_list(AllocatedNode **heap_arena_head) {
    AllocatedNode *head = (*heap_arena_head);

    while (head) {
        AllocatedNode *t = head;
        head->prev = NULL;
        head = head->next;
        
        free(t->payload);
        t->payload = NULL;
        free(t);
        t = NULL;
    }
}

void delete_segregated_list(SegregatedFreeList **segregated_free_list) {
    SegregatedListNode *head = (*segregated_free_list)->head;

    while (head != NULL) {
        SegregatedListNode *t = head;
        head->prev = NULL;
        head = head->next;
        
        free(t->data);
        t->data = NULL;
        
        free(t);
        t = NULL;
    }

    free((*segregated_free_list)->data);
    (*segregated_free_list)->data = NULL;

    free((*segregated_free_list));
    (*segregated_free_list) = NULL;
}

// DESTROYING HEAP LOGIC SECTION
void destroy_heap(HeapStructure **main_heap, HeapArena **heap_arena) {
    for (int i = 0; i < (*main_heap)->number_of_lists; i++) {
        delete_segregated_list(&(*main_heap)->segregated_free_lists[i]);
    }

    free((*main_heap)->segregated_free_lists);
    free((*main_heap));
    *main_heap = NULL;

    delete_heap_arena_list(&(*heap_arena)->head);
    free((*heap_arena));
}

void retrieve_write_parameters(char *line, int *address, int *bytes, char **message) {
    char *duplicated_line = calloc(strlen(line) + 1, 1);
    char *size = calloc(strlen(line) + 1, 1);
    char *size_saved = size;

    strcpy(duplicated_line, line);
    strcpy(size, line);

    // looking for the payload
    char *payload = strchr(duplicated_line, '"');
    payload += 1;
    char *payload_end = strrchr(duplicated_line, '"');
    int position = payload_end - payload;
    payload[position] = '\0';

    // looking for the number of bytes
    size_saved = strrchr(size_saved, ' ');
    size_saved++;
    *bytes = atoi(size_saved);

    // saving the payload in the returning variable, message
    *message = calloc(strlen(line) + 1, 1);
    strcpy(*message, payload);

    // looking for address
    char *word = strtok(line, " ");
    word = strtok(NULL, " ");
    *address = strtol(word, NULL, 0);

    // freeing the local resources
    free(duplicated_line);
    free(size);
}

void retrieve_read_parameters(char *line, int *address, int *bytes) {
    char *word = strtok(line, " ");
    word = strtok(NULL, " ");

    *address = strtol(word, NULL, 0);
    word = strtok(NULL, " ");

    *bytes = atoi(word);
}


// writing function, including edge cases
int write(HeapArena **heap_arena, HeapStructure **main_heap, char *line) {
    int requested_address, bytes;
    char *message = NULL;
    int segfaulted = 0;

    retrieve_write_parameters(line, &requested_address, &bytes, &message);

    AllocatedNode *head = (*heap_arena)->head;
    int is_segmentation_fault = 1;

    if (head != NULL) {
        int start_address = 0;
        AllocatedNode *left_bound = head, *right_bound = head;

        int total_size = 0;
        if (head->next == NULL) {
            int universal_counter = 0;
            if (head->start_address <= requested_address && (requested_address + strlen(message)) < (head->start_address + head->size)) {
                int difference = requested_address - head->start_address;
                start_address = head->start_address;

                for (int i = difference; i < head->size && universal_counter < strlen(message); i++) {
                    head->payload[i] = message[universal_counter++];
                }
            }
        } else {
            while (head -> next != NULL) {
                int local_start_address = head->start_address;
                
                if (requested_address >= local_start_address && requested_address < local_start_address + head->size) {
                    start_address = local_start_address;
                    left_bound = head;
                    total_size += head->size;
                }

                if (start_address != 0) {
                    if (head->next->start_address == local_start_address + head->size) {
                        // pot extinde zona unde as putea ipotetic sa scriu sirul de caractere
                        right_bound = head->next;
                        total_size += head->next->size;
                    } else if (total_size < strlen(message)) {
                        start_address = 0;
                        break;
                    }
                }
           
                head = head->next;
            }
        }

        int universal_counter = 0;
        if (left_bound != NULL && start_address) {
            for (AllocatedNode *iterator = left_bound; iterator != right_bound->next; iterator = iterator->next) {
                if (iterator == left_bound) {
                    int difference = requested_address - start_address;
                    
                    for (int i = difference; i < iterator->size && universal_counter < strlen(message); i++) {
                        iterator->payload[i] = message[universal_counter++];
                    }
                } else {
                    for (int i = 0; i < iterator->size && universal_counter < strlen(message); i++) {
                        iterator->payload[i] = message[universal_counter++];
                    }
                } 
            }
        }

        if (!start_address) {
            printf("Segmentation fault (core dumped)\n");
            segfaulted = 1;
            dump_memory(heap_arena, main_heap);
        }
    }

    free(message);
    return segfaulted;
}

// reading function, including edge cases 
int read(HeapArena **heap_arena, HeapStructure **main_heap, char *line) {
    int requested_address, bytes;
    int segfaulted = 0;
    retrieve_read_parameters(line, &requested_address, &bytes);

    AllocatedNode *head = (*heap_arena)->head;
    int is_segmentation_fault = 1;

    char *message = calloc(bytes + 1, 1);
    if (head != NULL) {
        int start_address = 0;
        AllocatedNode *left_bound = head, *right_bound = head;

        int total_size = 0;
        if (head->next == NULL) {
            int universal_counter = 0;
            if (head->start_address <= requested_address && (requested_address + bytes) < (head->start_address + head->size)) {
                int difference = requested_address - head->start_address;
                start_address = head->start_address;

                for (int i = difference; i < head->size && universal_counter < bytes; i++) {
                    message[universal_counter++] = head->payload[i];
                }
            }
        } else {
            while (head -> next != NULL) {
                int local_start_address = head->start_address;
                
                if (requested_address >= local_start_address && requested_address < local_start_address + head->size) {
                    start_address = local_start_address;
                    left_bound = head;
                    total_size += head->size;
                }

                if (start_address != 0) {
                    if (head->next->start_address == local_start_address + head->size) {
                        right_bound = head->next;
                        total_size += head->next->size;
                    } else if (total_size < bytes) {
                        start_address = 0;
                        break;
                    }
                }
           
                head = head->next;
            }
        }

        int universal_counter = 0;
        if (left_bound != NULL && start_address) {
            for (AllocatedNode *iterator = left_bound; iterator != right_bound->next; iterator = iterator->next) {
                if (iterator == left_bound) {
                    int difference = requested_address - start_address;
                    
                    for (int i = difference; i < iterator->size && universal_counter < bytes; i++) {
                        message[universal_counter++] = iterator->payload[i];
                    }
                } else {
                    for (int i = 0; i < iterator->size && universal_counter < bytes; i++) {
                        message[universal_counter++] = iterator->payload[i];
                    }
                } 
            }
        }

        if (!start_address) {
            printf("Segmentation fault (core dumped)\n");
            segfaulted = 1;
            dump_memory(heap_arena, main_heap);
        } else if (message != NULL) {
            message[strlen(message)] = 0;
            printf("%s\n", message);
        }
    }

    free(message);
    return segfaulted;
}

void homework_loop(char *line, HeapArena **heap_arena, HeapStructure **main_heap) {
    int segfault = 0;
    while (fgets(line, 999, stdin)) {
        if (!strncmp(line, "INIT_HEAP", strlen("INIT_HEAP"))) {
            create_heap(heap_arena, main_heap, line);
        } else if (!strncmp(line, "MALLOC", strlen("MALLOC"))) {
            handle_malloc(heap_arena, main_heap, line);
        } else if (!strncmp(line, "DUMP_MEMORY", strlen("DUMP_MEMORY"))) {
            dump_memory(heap_arena, main_heap);
        } else if (!strncmp(line, "FREE", strlen("FREE"))) {
            handle_free(heap_arena, main_heap, line);
        } else if (!strncmp(line, "READ", strlen("READ"))) {
            segfault = read(heap_arena, main_heap, line);
            if (segfault) {
                destroy_heap(main_heap, heap_arena);
                break;
            }
        } else if (!strncmp(line, "WRITE", strlen("WRITE"))) {
            segfault = write(heap_arena, main_heap, line);
            if (segfault) {
                destroy_heap(main_heap, heap_arena);
                break;
            }
        } else if (!strncmp(line, "DESTROY_HEAP", strlen("DESTROY_HEAP"))) {
            destroy_heap(main_heap, heap_arena);
        }
    }
}

int main(int argc, char **argv) {
    char *line = (char *) malloc(sizeof(*line) * 1000);
    
    HeapStructure *main_heap = NULL;
    HeapArena *heap_arena = NULL;
    
    homework_loop(line, &heap_arena, &main_heap);

    free(line);
    return 0;
}