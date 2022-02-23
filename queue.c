/*
 *  queue.c
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

#include "hardware.h"
#include "ylib.h"
#include "process.h"
#include "queue.h"


/**
 * @brief create and return a queue node
 * 
 * @param id id of new node (most often process id)
 * @param data data to be contained in node
 * @return qnode_t* 
 * - NULL if failure
 * - new queue node otherwise
 */
static qnode_t *qnode_init(int id, pcb_t *data);

/**
 * @brief delete a queue node
 * 
 * @param node queue node to be deleted
 * @param dataDelete function pointer to a function to delete data in node
 */
static void qnode_delete(qnode_t * node, void (*dataDelete) (pcb_t *data));


// QUESTION: do we need a function for deleting queues?


/**
 * @brief initializes the queue at the inner pointer
 * 
 * @param queue pointer to queue pointer
 * @return int 
 *  - 0 if succesful 
 *  - 1 otherwise
 */
queue_t * queue_init() {
    queue_t *queue = malloc(sizeof(queue_t));
    if (queue == NULL ) return NULL;
    queue->id = 0; 
    queue->size = 0; 
    queue->head = NULL; 
    queue->tail = NULL;
    return queue;
}

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
int queue_add(queue_t *queue, pcb_t *data, int  id) {
    // ensure all the args are valid
    if (queue == NULL || data == NULL) return -1;
    qnode_t *node = qnode_init(id, data);
    if (node == NULL) return -1;
    if (queue->head == NULL) {
        queue->head = node; // set head as new node
        queue->tail = node; // set tail as new node
    } else { // otherwise queue is not empty
        queue->tail->next = node;
        queue->tail =  queue->tail->next;
    }
    queue->size++;  // increment queue size
    return 0; 
}

/**
 * @brief 
 * 
 * @param queue removes item from the given queue
 * @param id id of data to remove
 * @return pcb_t * 
 * - NULL if failure
 * - data with id in queue if success
 */
pcb_t *queue_remove(queue_t *queue, int id) {
}

/**
 * @brief removes process at the front of the queue
 * and return the pointer to the removed process
 * 
 * @param queue pointer to queue to pop from
 * @return pcb_t * pointer to data at front of queue
 */
pcb_t *queue_pop(queue_t *queue) {
    if (queue == NULL) return NULL;
    if (queue->head == NULL) return NULL;
    qnode_t *node = queue->head;
    queue->head = node->next;
    if (queue->head == NULL) queue->tail = NULL;
    queue->size--;
    void *data = node->data; // get data from node
    qnode_delete(node, NULL);  // delete node withouth deleting node data
    return data;
}

/**
 * @brief finds if given queue contains item with given id
 * 
 * @param queue pointer to queue
 * @param id id of item to find in queue
 * @return int 
 * - 0 if not exists
 * - 1 if exists
 */
int queue_find(queue_t *queue, int id) {
}

/**
 * @brief delete a queue
 * 
 * @param queue queue to delete 
 * @param dataDelete function pointer to a function to delete data in queue
 */
void queue_delete(queue_t *queue, void (*dataDelete) (pcb_t *data)) {
}

/**
 * @brief create and return a queue node
 * 
 * @param id id of new node (most often process id)
 * @param data data to be contained in node
 * @return qnode_t* 
 * - NULL if failure
 * - new queue node otherwise
 */
static qnode_t *qnode_init(int id, pcb_t *data) {
    if (data == NULL) return NULL;
    qnode_t *node = malloc(sizeof(qnode_t));
    if (node == NULL) return NULL;
    node->id = id;
    node->data = data;
    node->next = NULL;
    return node;
}

/**
 * @brief delete a queue node
 * 
 * @param node queue node to be deleted
 * @param dataDelete function pointer to a function to delete data in node
 */
static void qnode_delete(qnode_t * node, void (*dataDelete) (pcb_t *data)) {
    if (node != NULL) {
        if (dataDelete != NULL) {
            dataDelete(node->data);
        }
        free(node);
    }
}
