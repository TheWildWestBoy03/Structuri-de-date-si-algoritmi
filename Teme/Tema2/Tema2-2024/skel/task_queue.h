#include <stdio.h>
#include "list.h"

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

typedef struct task_queue {
    list_t *tasks_list;
    unsigned int size;
} task_queue;

void add_task_in_queue(void *new_request, task_queue **queue, unsigned int datasize);
task_queue *init_task_queue(unsigned int datasize);
list_node_t *extract_the_front_node(task_queue *queue);
void remove_the_front_task(task_queue **queue);

#endif