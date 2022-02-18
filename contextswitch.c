/*
 *  contextswitch.c
 *  
 *  holds the functions KCCopy and KCSwitch
*/

#include <hardware.h>
#include <ykernel.h>
#include "process.h"

extern int *ptr_bit_vector;

/**
 * @brief to allow for future context switches, clones kernel stack and kernel context
 * 
 * @param kctxt 
 * @param currPCB 
 * @param newPCB 
 * @return KernelContext* 
 */
KernelContext *KCCopy(KernelContext *kc_in, void *pcb, void *notUsed) {
    TracePrintf(0,"Entered KCCopy!\n");

    if (kc_in == NULL || pcb == NULL) {
        TracePrintf(0,"We got null kernel context and/or null pcb\n");
        return NULL;
    }

    
    pcb_t *currPCB = (pcb_t *) pcb;

    // copy kernel context
    memcpy(&(currPCB->kernel_context), kc_in, sizeof(KernelContext));

    // lowest valid stack virtual page number
    int kernel_stack_base = ( (int)KERNEL_STACK_BASE >> PAGESHIFT );
    
    // first invalid virtual page above stack
    int kernel_base = (int)VMEM_0_BASE >> PAGESHIFT;

    
    pte_t *_dst = currPCB->kernel_stack_pt;

    // src is region0 page table
    pte_t *_src = ( pte_t *) ReadRegister(REG_PTBR0);  
    int allocated = 0;

    // looping through kernel stack
    for (int index = kernel_stack_base; index >= kernel_base && allocated < KERNEL_STACK_SIZE; index--) {
        // if(_src[index])

        // if we find a free page
        if (ptr_bit_vector[index] == PAGE_FREE) {

            // setting up a dummy page table
            _src[index].pfn = _dst[allocated].pfn;
            _src[index].valid = VALID_FRAME;
            _src[index].prot = NO_X_W_R;

            // copy data
            void *to = (void *) (index << PAGESHIFT);
            void *from = (void *) ( (kernel_stack_base + allocated) << PAGESHIFT );
            TracePrintf(0, "d_index ->%d s_index -> %d\n", _dst[allocated].pfn, kernel_stack_base + allocated);
            // WriteRegister(REG_TLB_FLUSH, (unsigned int) to);
            memcpy(to, from, PAGESIZE);

            // clear dummy page table entry
            _src[index].pfn = 0;
            _src[index].valid = 0;
            _src[index].prot = 0;

            // move onto next page in stack (there's 2 total)
            allocated++;
        }
    }
    // after this, flush region0
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    return kc_in;
}

/**
 * @brief switches from pcb1 to pcb2
 * 
 * @param kc_in kernel context
 * @param pcb1 
 * @param pcb2 
 * @return KernelContext* 
 */
KernelContext *KCSwitch(KernelContext *kc_in, void *pcb1, void *pcb2) {

    pcb_t *curr = (pcb_t *) pcb1;
    pcb_t *next = (pcb_t *) pcb2;

    TracePrintf(0,"KCSwitch called! curr process id: %d\n",(int)(curr->pid));

    // copy kernel context
    KernelContext tmp = *kc_in;
    memcpy(&(curr->kernel_context), kc_in, sizeof(KernelContext));

    pte_t *k_pt = (pte_t *) ReadRegister(REG_PTBR0);


    // iterate through next_pcb's kernel stack and update region0 pagetable
    int stack_base = ( (int) KERNEL_STACK_BASE >> PAGESHIFT );


    // change the pte to point to the same stack framme
    for (int i = 0 ; i < KERNEL_STACK_SIZE; i++ ) {
        TracePrintf(0, "p_i -> %d k_i -> %d p_pfn -> %d k_pfn -> %d \n", i, stack_base + i,  next->kernel_stack_pt[i].pfn, k_pt[stack_base + i].pfn);
        k_pt[stack_base + i].pfn = next->kernel_stack_pt[i].pfn;
        k_pt[stack_base + i].prot = next->kernel_stack_pt[i].prot;
        k_pt[stack_base + i].valid = next->kernel_stack_pt[i].valid;
    }
    // Pause();
    
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    // update ptbr0
    WriteRegister(REG_PTBR0, (unsigned int) k_pt);
    TracePrintf(0,"done with KCSwitch, returning next_pcb kernel context\n");

    // return pointer to KernelContext in new PCB
    return &(next->kernel_context);
}