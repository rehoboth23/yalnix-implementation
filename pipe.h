#ifndef __PIPE_H_
#define __PIPE_H_

#include "yalnix.h"
#include "queue.h"

typedef struct pipe {

    char buf[PIPE_BUFFER_LEN];
    int plen;
    int id;
    struct pipe *next;
    int being_used;     // flag variable, whether or not the pipe is being used
    queue_t *queue;     // queue of processes waiting to read or write

} pipe_t;



/* function to initialize pipe */
pipe_t* init_head_pipe();

int add_pipe(pipe_t *head_pipe);

int remove_pipe(pipe_t *head_pipe, int id);

pipe_t* get_pipe(pipe_t* head_pipe, int id);

#endif
