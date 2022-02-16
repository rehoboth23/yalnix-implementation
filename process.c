/*
 *  process.c
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#include <stdio.h>
#include <stdlib.h>
#include "process.h"

// typedef struct PCB { 
//     u_long pid;
//     unsigned long child_pids[];            // array of child pids
//     int num_children;               // number of children
//     pcb_t* parent;                  // pointer to parent

//     UserContext *user_context;      // hardware.h provides UserContext
//     KernelContext *kernel_context;  // hardware.h provides KernelContext

//     // user address space info, this is the FIRST ADDRESS of each segment
//     pte_t **user_page_table; // region 1

//     /*void *user_heap_addr;           
//     void *user_text_segment_addr;
//     void *user_data_segment; */

//     int user_stack_pt_index;
//     int user_heap_pt_index;
//     int user_text_pt_index;
//     int user_data_pt_index;

//     // kernel address space info, this is the FIRST ADDRESS of each segment
//     pte_t **kernel_page_table;

//     /*void *kernel_heap_addr;
//     void *kernel_text_segment_addr;
//     void *kernel_data_segment; */
//     int kernel_stack_pt_index;
//     int kernel_heap_pt_index;
//     int kernel_text_pt_index;
//     int kernel_data_pt_index;

//     int status;                      // identifies whether process is alive or dead

// } pcb_t; */

pcb_t *init_process() {
    pcb_t *process = malloc(sizeof(pcb_t));

    if (process == NULL) {
        TracePrintf(0,"Couldn't malloc for init process\n");
        return NULL;
    }

    // initialize all values to NULL or zero
    process->pid = 0;
    process->child_pids = NULL;
    process->num_children = 0;
    //process->parent = NULL;

    process->user_context = NULL;
    process->kernel_context = NULL;
    process->user_page_table = NULL;
    process->user_stack_pt_index = 0;
    process->user_heap_pt_index = 0;
    process->user_text_pt_index = 0;
    process->user_data_pt_index = 0;
    process->kernel_page_table = NULL;
    process->kernel_stack_pt_index = 0;
    process->kernel_heap_pt_index = 0;
    process->kernel_text_pt_index = 0;
    process->kernel_data_pt_index = 0;
    
    process->delay_clock = 0;

    // initialize process stack frames to 0
    for (int i = 0; i < KERNEL_STACK_MAXSIZE/PAGESIZE; i++) {
        process->kernel_stack_frames[i] = 0;
    }

    return process;
}