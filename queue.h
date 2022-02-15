/*
 *  queue.h
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

#include "process.h"

typedef struct Queue {
    // malloc space in heap
    // check malloc, if not enough space, give error

    // return address to malloc'd memory
} queue_t;



int queue_init(queue_t **queue);
int queue_add(queue_t *queue, pcb_t *process);
int queue_remove(queue_t *queue, pcb_t *process);
pcb_t* queue_pop(queue_t *queue);
int queue_find(queue_t *queue, pcb_t *process);
int queue_size(queue_t *queue);
