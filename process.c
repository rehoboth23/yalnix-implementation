/*
 *  process.c
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#include "hardware.h"
#include "process.h"

typedef struct PCB { 
    pte_t *page_table;
    u_long pid;
    unsigned long child_pids[];            // array of child pids
    int num_children;               // number of children
    pcb_t* parent;                  // pointer to parent

    UserContext *user_context;      // hardware.h provides UserContext
    KernelContext *kernel_context;  // hardware.h provides KernelContext

    // user address space info, this is the FIRST ADDRESS of each segment
    void *user_heap_addr;           
    void *user_text_segment_addr;
    void *user_data_segment;

    // kernel address space info, this is the FIRST ADDRESS of each segment
    void *kernel_heap_addr;
    void *kernel_text_segment_addr;
    void *kernel_data_segment;

    int status                      // identifies whether process is alive or dead

} pcb_t;

int process_init(pcb_t **processBlock) {
    // allocate memory for pcb at inner pointer of processBlock
    // allocated memory for variables, give error if fail

    // look at current processes(?) and generate pid

    // initialize page table
    // look at memory and assign addresses

    // inherit kernel heap, txt, data from other process

    // initialize usercontext and kernel context (check hardware.h)
    // return 0 if success or {x, y|x< 0, y > 0} if fail
}