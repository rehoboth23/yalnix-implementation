/*
 *  queue.c
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

#include "queue.h"
#include <ylib.h>

/*
 *  queue.c
 *  
 *  holds functions that handle queues, an array of pointers to processes
 *  this'll be the helper-functions for our running, ready, defunct, and blocked queues
*/

/**
 * @brief node in a queue
 * 
 */
typedef struct qnode {
    int id;
    pcb_t *data;
    struct qnode *next;
} qnode_t;


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


/**
 * @brief general purpose Queue struct for yalnix OS
 * 
 */
typedef struct Queue {
    int id;
    int size;
    qnode_t *head;
    qnode_t *tail;
} queue_t;

// QUESTION: do we need a function for deleting queues?


/**
 * @brief initializes the queue at the inner pointer
 * 
 * @param queue pointer to queue pointer
 * @return int 
 *  - 0 if succesful 
 *  - 1 otherwise
 */
int queue_init(queue_t *queue) {
    queue = malloc(sizeof(queue_t));
    if ( queue == NULL ) {
        return 1; 
        }
    queue->id = 0; 
    queue->size = 0; 
    (queue)->head = NULL; 
    (queue)->tail = NULL; 
    return 0;
}

/**
 * @brief sets the id of a queue
 * 
 * @param queue pointer to queue
 * @param id value of id
 */
void queue_set_id(queue_t *queue, int id) {
    queue->id = id;
}

/**
 * @brief gets the id of a queue
 * 
 * @param queue pointer to queue
 * @return int id of queue
 * - queue is null return -1
 */
int queue_get_id(queue_t *queue) {
    if (queue == NULL) return -1;
    return queue->id;
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
    if (queue == NULL || data == NULL) return 1;
    
    // init node for new item
    qnode_t *node = qnode_init(id, data);
    
    if (node == NULL) return 1;

    if (queue->size == 0) { // if queue is empty
        queue->head = node; // set head as new node
        queue->tail = queue->head;  // set tail same value as head
    } else {    // otherwise queue is not empty
        queue->tail->next = node;   // set tail next to new node
        queue->tail = node; // set tail to new node
    }
    queue->size += 1;   // increase queue size
    return 0; 
}

/**
 * @brief 
 * 
 * @param queue removes item from the given queue
 * @param id id of data to remove
 * @return void* 
 * - NULL if failure
 * - data with id in queue if success
 */
void *queue_remove(queue_t *queue, int id) {

    if (queue == NULL) return NULL; // if arg is in valid
    if (queue->size == 0) return NULL;  // if queue is empty

    qnode_t *node = queue->head;    // get queue head
    void *data = NULL;  // return value

    if (node->id == id) { // if head is what we are looking for
        data = node->data;  // get node data

        if (queue->size > 1) {  // if queue has more than 1 item
            queue->head = node->next;   // set head to next of head

        } else {    // otherwisw queue conotains only head
            queue->head = NULL; // set queue head to null
            queue->tail = NULL; // set queue tail to null
        }

        qnode_delete(node, NULL);   // delete node withouth deleting node data
    }

    // current node is head ; if there is only head loop never runs
    // we are checking next node of current node to keeep reference to current node
    for (int i = 1; i < queue->size; i++) { // loop until just before tail
        if (node->next == NULL) break;  // if node next is null break

        if (node->next->id == id) { // if next node is what we are looking for
            data = node->next->data;    // get node data
            qnode_t *todel = node->next; // keep reference to node next

            if (i + 1 == queue->size) { // if next node is tail (last node)
                queue->tail = NULL; // set queue tail to null
                node->next = NULL;  // set node next to null

            } else {    // otherwise if node next is not last node
                node->next = todel->next;  // set node next to next of node next
            }

            qnode_delete(todel, NULL); // delete node withouth deleting node data
            break;
        }
        node = node->next; // move to next node
    }

    if (data != NULL) queue->size -= 1; // reduce queue size if node was added

    return data;
}

/**
 * @brief removes process at the front of the queue
 * and return the pointer to the removed process
 * 
 * @param queue pointer to queue to pop from
 * @return void * pointer to data at front of queue
 */
void *queue_pop(queue_t *queue) {

    if (queue == NULL) return NULL;
    if (queue->size == 0) return NULL;

    qnode_t *node = queue->head;

    if (queue->size == 1) { // if queue has only one item
        queue->head = NULL; // set queue head as null
        queue->tail = NULL; // set queue tail as null

    } else queue->head = node->next;
    
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
    if (queue == NULL) return 0;
    if (queue->size == 0) return 0;

    for (qnode_t *node = queue->head; node != NULL; node = node->next) {
        if (node ->id == id) return 1;
    }
    return 0;
}

/**
 * @brief get size of the queue
 * 
 * @param queue 
 * @return int 
 * size of queu
 * -1 if something went wrong.
 */
int queue_size(queue_t *queue) {
    return queue == NULL ? -1 : queue->size;
}

/**
 * @brief delete a queue
 * 
 * @param queue queue to delete 
 * @param dataDelete function pointer to a function to delete data in queue
 */
void queue_delete(queue_t *queue, void (*dataDelete) (void *data)) {
    if (queue != NULL) {
        for (qnode_t *node = queue->head; node != NULL;) {
            qnode_t *next = node->next;
            qnode_delete(node, dataDelete);
            node = next;
        }
    }
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
