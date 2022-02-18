/*
 *  process.h
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "hardware.h"
#include "include.h"

typedef struct PCB {
    u_long pid;
    unsigned long *child_pids;            // array of child pids
    int num_children;               // number of children
    //pcb_t* parent;      maybe not a pointer pcb_t because this won't work here... at least in header            // pointer to parent

    UserContext user_context;      // hardware.h provides UserContext
    KernelContext kernel_context;  // hardware.h provides KernelContext

    // user address space info, this is the FIRST ADDRESS of each segment
    pte_t user_page_table[USER_PT_SIZE]; // region 1

    int user_stack_pt_index;
    int user_heap_pt_index;
    int user_text_pt_index;
    int user_data_pt_index;

    // kernel address space info, this is the FIRST ADDRESS of each segment
    pte_t kernel_stack_pt[KERNEL_STACK_SIZE];

    int status;                      // identifies whether process is alive or dead} pcb_t;
} pcb_t;
// NOTE: I'm not sure about this function, we definitely need to
// initialize a lot of stuff, but the details are quite hard to

pcb_t *init_process();

#endif