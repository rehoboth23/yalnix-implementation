/*
 *  contextswitch.c
 *  
 *  holds the functions KCCopy and KCSwitch
*/


#include <hardware.h>
#include <ykernel.h>
#include "process.h"

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *not_used) {

}

KernelContext *KCSwitch(KernelContext *kc_in,void *curr_pcb_p, void *next_pcb_p) {

    TracePrintf(0,"KCSwitch called! curr process id: %d\n",(int)((pcb_t *)curr_pcb_p)->pid);

// copy kernel context to old pcb
    KernelContext tmp = *kc_in;
    ((pcb_t *)curr_pcb_p)->kernel_context = &tmp;

// change region 0 stack mappings to that for new PCB
    // change stack pointer
    ((pcb_t *)next_pcb_p)->kernal_stack_pt_index = ((pcb_t *)curr_pcb_p)->kernal_stack_pt_index;

    // iterate through curr_pcb's stack and map kernel stack ptable entries
    
    // update the page tables

// return pointer to KernelContext in new PCB
    return ((pcb_t *)next_pcb_p)->kernel_context;
    
}


