/*
 * pipe.c
 *
 * The format of the file system and other constants used by YFS
 */

#include "ylib.h"
#include "pipe.h"
#include "kernel.h"
#include "include.h"


// /* see pipe.h for more information */
/**
 * @brief initializes the head of the linked list of pipes
 * 
 * @return pipe_t* 
 */
pipe_t * init_head_pipe() {
    // malloc memory
    pipe_t *pipe = malloc(sizeof(pipe_t));
    if (pipe == NULL) {
        TracePrintf(0,"ERROR: init_head_pipe failed malloc\n");
        return NULL;
    }

    // initialize variables
    pipe->id = MAX_LOCKS + MAX_CVARS + 1; // pipe ids will be at the end of these things
    pipe->next = NULL;
    pipe->plen = 0;
    pipe->being_used = PIPE_FREE;
    memset(pipe->buf,0,PIPE_BUFFER_LEN);
    if (pipe->buf == NULL) {
        TracePrintf(0,"ERROR: init_head_pipe failed malloc for buf\n");
        return NULL;
    }
    return pipe;
}

/**
 * @brief adds pipe to the given linked list of pipes
 * 
 * @param head_pipe pointer to the head of linked list
 * @return int , id of new pipe, otherwise ERROR
 */
int add_pipe(pipe_t *head_pipe) {
    // malloc memory
    pipe_t *new_pipe = malloc(sizeof(pipe_t));
    if (new_pipe == NULL) {
        TracePrintf(0,"ERROR: new_pipe failed malloc\n");
        return ERROR;
    }
    if (head_pipe == NULL) {
        TracePrintf(0,"ERROR: head_pipe is null\n");
        return ERROR;
    }

    // initialize variables
    new_pipe->next = NULL;
    new_pipe->plen = 0;
    new_pipe->being_used = PIPE_FREE;
    memset(new_pipe->buf,0,PIPE_BUFFER_LEN);
    
    // initialize queue
    new_pipe->queue = queue_init();

    if (new_pipe->queue == NULL) {
        TracePrintf(0,"ERROR: add_pipe, queue initialization failed\n");
    }

    // get to the tail of the linked list
    pipe_t* curr_pipe = head_pipe;

    while(curr_pipe->next != NULL) {
        curr_pipe = curr_pipe->next;

    }

    TracePrintf(0,"currently at pipe %p, with id %d\n",curr_pipe,curr_pipe->id);

    // set up new next pipe and id of our new pipe
    curr_pipe->next = new_pipe;
    new_pipe->id = curr_pipe->id + 1;

    TracePrintf(0,"Creating pipe with id %d\n",new_pipe->id);

    return new_pipe->id; 
}

/**
 * @brief remove the pipe with id provided and update
 * linked list of pipes accordingly
 * 
 * @param head_pipe 
 * @param id of pipe to remove
 * @return int, error code, 0 if success, ERROR if not
 */
int remove_pipe(pipe_t *head_pipe, int id) {
    
    // find the pipe before the pipe we want to remove
    pipe_t* curr_pipe = head_pipe;
    pipe_t* pipe_before;

    // Interate through the list of pipes to find the pipe with id
    while (curr_pipe->id != id) {
        if (curr_pipe->next == NULL) {
            TracePrintf(0,"ERROR: remove_pipe couldn't find pipe with id %d\n",id);
            return ERROR;
        }

        pipe_before = curr_pipe;
        curr_pipe = curr_pipe->next;
    }

    if (curr_pipe->next == NULL) {
        pipe_before->next = NULL;            // If current pipe is at the end of the list, set pipe before to null
    } else {
        pipe_before->next = curr_pipe->next; // If the current pipe is in the middle, set the pipe before to next pipe
    }

    free(curr_pipe);
    return 0;
}



/**
 * @brief Get the pipe object
 * 
 * @param head_pipe 
 * @param id 
 * @return pipe_t* 
 */
pipe_t* get_pipe(pipe_t* head_pipe, int id) {
    pipe_t* curr_pipe = head_pipe;

    while(curr_pipe->id != id) {

        if (curr_pipe->next == NULL) {
            TracePrintf(0,"ERROR: read_pipe couldn't find pipe %d\n",id);
            return NULL;
        }

        curr_pipe = curr_pipe->next;
    }

    return curr_pipe;
}