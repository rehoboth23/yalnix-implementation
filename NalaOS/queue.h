#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "process.h"

/**
 * @brief node in a queue
 * 
 */
typedef struct qnode {
    int id;
    pcb_t *data;
    struct qnode *next;
} qnode_t;

typedef struct queue {
    int id;
    int size;
    qnode_t *head;
    qnode_t *tail;
} queue_t;

/**
 * @brief initializes the queue at the inner pointer
 * 
 * @return queue_t * 
 *  - new queue on success
 *  - NULL otherwise
*/
queue_t * queue_init();

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
 * @return pcb_t* 
 * - NULL if failure
 * - data with id in queue if success
 */
pcb_t *queue_remove(queue_t *queue, int id);

/**
 * @brief removes process at the front of the queue
 * and return the pointer to the removed process
 * 
 * @param queue pointer to queue to pop from
 * @return pcb_t * pointer to data at front of queue
 */
pcb_t *queue_pop(queue_t *queue);

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
 * @brief function to peek at the head of a queue
 * 
 * @param queue the queue to peek into
 * @return pcb_t* head of the queue
 */
pcb_t *queue_peek(queue_t *queue);

/**
 * @brief delete a queue
 * 
 * @param queue queue to delete 
 * @param dataDelete function pointer to a function to delete data in queue
 */
void queue_delete(queue_t *queue, void (*dataDelete) (pcb_t *data));

#endif