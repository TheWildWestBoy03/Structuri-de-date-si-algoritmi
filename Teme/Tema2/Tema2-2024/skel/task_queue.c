#include <stdio.h>
#include "task_queue.h"

void add_task_in_queue(void *new_request, task_queue **queue, unsigned int datasize) {
    add_node(new_request, &(*queue)->tasks_list->head, &(*queue)->tasks_list->tail, datasize);
    (*queue)->size ++;
}

task_queue* init_task_queue(unsigned int datasize) {
    task_queue* new_task_queue = malloc(sizeof(task_queue));
    new_task_queue->size = 0;
    create_list(datasize, &new_task_queue->tasks_list, datasize);

    return new_task_queue;
}

list_node_t *extract_the_front_node(task_queue *queue) {
    return return_oldest_node(queue->tasks_list);
}

void remove_the_front_task(task_queue **queue) {
    remove_node(&(*queue)->tasks_list->head, &(*queue)->tasks_list->tail);
    (*queue)->size--;
}
