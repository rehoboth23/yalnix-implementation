/*
 *  contextswitch.c
 *  
 *  holds the functions KCCopy and KCSwitch
*/


#include <hardware.h>
#include <ykernel.h>
#include "process.h"

extern pte_t *ptr_k_pt;

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *not_used) {

}

KernelContext *KCSwitch(KernelContext *kc_in,void *curr_pcb_p, void *next_pcb_p) {

    TracePrintf(0,"KCSwitch called! curr process id: %d\n",(int)((pcb_t *)curr_pcb_p)->pid);

// copy kernel context to old pcb
    KernelContext tmp = *kc_in;
    ((pcb_t *)curr_pcb_p)->kernel_context = &tmp;

// change region 0 stack mappings to that for new PCB

    // QUESTION: do we change stack pointer? (the line right below this)
    // ((pcb_t *)next_pcb_p)->kernel_stack_pt_index = ((pcb_t *)curr_pcb_p)->kernel_stack_pt_index;

    // iterate through next_pcb's kernel stack and update region0 pagetable
    int stack_index = (int)KERNEL_STACK_BASE >> PAGESHIFT;
    int counter = 0;

    // change the pte to point to the same stack framme
    for (stack_index ; stack_index < KERNEL_STACK_LIMIT >> PAGESHIFT; stack_index++ ) {

        TracePrintf(0,"in KCSwitch: changing kernel page table from %d to %d...\n",ptr_k_pt[stack_index].pfn, ((pcb_t *)next_pcb_p)->kernel_stack_frames[counter]);
        
        // page frame number of in page table for kernel stack is set to process B's frame
        (ptr_k_pt[stack_index]).pfn = ((pcb_t *)next_pcb_p)->kernel_stack_frames[counter];
        counter++;
    }
    
    TracePrintf(0,"done with KCSwitch, returning next_pcb kernel context\n");

    // return pointer to KernelContext in new PCB
    return ((pcb_t *)next_pcb_p)->kernel_context;
}


