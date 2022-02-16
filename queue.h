/*
 *  queue.h
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

#include "process.h"

/**
 * @brief general purpose Queue struct for yalnix OS
 */
typedef struct Queue queue_t;


/**
 * @brief initializes the queue at the inner pointer
 * 
 * @param queue pointer to queue pointer
 * @return int 
 *  - 0 if succesful 
 *  - 1 otherwise
 */
int queue_init(queue_t **queue);

/**
 * @brief sets the id of a queue
 * 
 * @param queue pointer to queue
 * @param id value of id
 */
void queue_set_id(queue_t *queue, int id);

/**
 * @brief gets the id of a queue
 * 
 * @param queue pointer to queue
 * @return int id of queue
 */
int queue_get_id(queue_t *queue);

/**
 * @brief adds item to the given queue
 * 
 * @param queue pointer to queue
 * @param item item to add to queue
 * @param id    id of item to add to queue
 * @return int 
 *  - 0 if success
 *  - 1 if fail
 */
int queue_add(queue_t *queue, pcb_t *data, int  id);

/**
 * @brief 
 * 
 * @param queue removes item from the given queue
 * @param id id of data to remove
 * @return void* 
 * - NULL if failure
 * - data with id in queue if success
 */
void *queue_remove(queue_t *queue, int id);

/**
 * @brief removes process at the front of the queue
 * and return the pointer to the removed process
 * 
 * @param queue pointer to queue to pop from
 * @return void * pointer to data at front of queue
 */
void *queue_pop(queue_t *queue);

/**
 * @brief finds if given queue contains item with given id
 * 
 * @param queue pointer to queue
 * @param id id of item to find in queue
 * @return int 
 * - 0 if not exists
 * - 1 if exists
 */
int queue_find(queue_t *queue, int id);

/**
 * @brief get size of the queue
 * 
 * @param queue 
 * @return int 
 * size of queu
 * -1 if something went wrong.
 */
int queue_size(queue_t *queue);

/**
 * @brief delete a queue
 * 
 * @param queue queue to delete 
 * @param dataDelete function pointer to a function to delete data in queue
 */
void queue_delete(queue_t *queue, void (*dataDelete) (void *data));
