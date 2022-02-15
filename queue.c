/*
 *  queue.c
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

#include "queue.h"



// QUESTION: do we need a function for deleting queues?

/*
 * queue_init
 *  args
 *  - pointer to queue pointer
 *  does
 *  - initializes the queue at the inner pointer
 * returns
 *  - 0 if succesful 1 otherwise
 */
int queue_init(queue_t **queue) {
    // initialize a queue
    // malloc space in heap
    // check malloc, if not enough space, give error
    // return address to malloc'd memory
}

/*
 * queue_add
 *
 * adds process to the given queue
 * returns
 *  - 0 if success
 *  - 1 if fail
 */
int queue_add(queue_t *queue, pcb_t *process) {
    // check if both arguments are NULL

    //  do we need to do any other checks to make sure process is valid>]?

    // call queue_find to check if it is already in the queue
        // return error if so, something went wrong
    
    // otherwise, add the process to the end of the array.
}

/*
 * queue_remove
 *
 * removes process from the given queue
 * returns
 *  - 0 if success
 *  - 1 if fail
 */
int queue_remove(queue_t *queue, pcb_t *process) {
    // check if both arguments are NULL/invalid

    // call queue_find 
    // if it returns -1, return 1 as something went wrong

    // otherwise, remove the process from the index, and shift all other processes
    // down the array

    // return 0
}

/*
 * queue_pop
 *
 * removes process at the front of the queue
 * and return the pointer to the removed process
 * returns
 *  - address to process if success
 *  - NULL if fail (e.g. queue is empty)
 */
pcb_t* queue_pop(queue_t *queue) {
    // give error if queue is empty

    // call queue_remove on the first process in array

    // return process
}

/*
 * queue_find
 *
 * finds given process in the given queue and gets the index
 * returns
 *  - index in array if it exists
 *  - -1 if it doesn't
 */
int queue_find(queue_t *queue, pcb_t *process) {
    // check if both arguments are NULL/invalid

    // loop through each element of array

    // if process found, return index

    // otherwise, return -1
}


/*
 * queue_size
 *
 * returns
 *  - size of the queue
 *  - -1 if something went wrong.
 */
int queue_size(queue_t *queue) {
    // check if NULL, return -1 if so

    // give 0 if queue is empty

    // loop through array until end of array, inc counter

    // return counter
}

