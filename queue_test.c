/*
 *  queue_test.c
 *  
 *  individually tests queue.c and its functions
 *  completely separate from yalnix, compiled and tested separately
 */

#include "queue.h"
#include "process.h"

int main() {
    queue_t *queue1;
    
    printf("initializing queue1...\n");

    if (queue_init(queue1) != 0) {
        printf("queue init failed!\n");
    }

    pcb_t* process1;
    process1 = init_process;

    printf("size of queue1 is %d\n",queue_size(queue1));

    printf("add process1 to queue1...\n");

    if (queue_add(queue1,process1,process1->pid) != 0) {
        printf("queue add failed!\n");
    }

    printf("size of queue1 is %d\n",queue_size(queue1));

}