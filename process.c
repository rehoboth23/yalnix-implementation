/*
 *  process.c
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#include "ylib.h"
#include "hardware.h"
#include "process.h"
#include "include.h"

pcb_t *init_process() {
    pcb_t *process = malloc(sizeof(pcb_t));

    if (process == NULL) {
        return NULL;
    }

    // initialize all values to NULL or zero
    process->pid = 0;
    process->child_pids = NULL;
    process->num_children = 0;
    process->status = 0;

    memset(&(process->user_context), 0, sizeof(UserContext));

    memset(&(process->kernel_context), 0, sizeof(KernelContext));

    process->user_stack_pt_index = 0;
    process->user_heap_pt_index = 0;
    process->user_text_pt_index = 0;
    process->user_data_pt_index = 0;

    memset(process->kernel_stack_pt, 0, sizeof(pte_t) * ( ( ( (int) KERNEL_STACK_LIMIT >> PAGESHIFT) - ((int) KERNEL_STACK_BASE >> PAGESHIFT ) ) ));
    return process;
}