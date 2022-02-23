#include <hardware.h>
#include <ylib.h>
#include <yuser.h>
#include "kernel.h"
#include "include.h"
#include "process.h"


/**
 * @brief 
 * 
 * @param kctxt 
 * @param currPCB 
 * @param newPCB 
 * @return KernelContext* 
 */
KernelContext *KCCopy(KernelContext *kc_in, void *pcb, void *notUsed) {
    if (kc_in == NULL || pcb == NULL) {
        return NULL;
    }
    pcb_t *currPCB = (pcb_t *) pcb;
    memcpy(&(currPCB->kernel_context), kc_in, sizeof(KernelContext));

    // lowest valid stack virtual page number
    int kernel_stack_base = ( (int)KERNEL_STACK_BASE >> PAGESHIFT );
    // first invalid virtual page above stack
    int kernel_base = (int)VMEM_0_BASE >> PAGESHIFT;
    pte_t *_dst = currPCB->kernel_stack_pt;
    pte_t *_src = ( pte_t *) ReadRegister(REG_PTBR0);  // current kernel stack should be regioin 0 stack

    int reserved_kernel_index = -1;
    for (int index =kernel_stack_base ; index >= kernel_base; index--) {
        if (_src[index].valid == INVALID_FRAME) {
            reserved_kernel_index = index;
            break;
        }
    }
    if (reserved_kernel_index == -1) {
        return NULL;
    }
    _src[reserved_kernel_index].valid = VALID_FRAME;
    _src[reserved_kernel_index].prot = NO_X_W_R;

    for (int allocated = 0; allocated < KERNEL_STACK_SIZE; allocated++) {
        _dst[allocated].valid = VALID_FRAME;
        _src[reserved_kernel_index].pfn = _dst[allocated].pfn;
        void *to = (void *) (reserved_kernel_index << PAGESHIFT);
        void *from = (void *) ( (kernel_stack_base + allocated) << PAGESHIFT );
        WriteRegister(REG_TLB_FLUSH, (unsigned int) to);
        memcpy(to, from, PAGESIZE);
    }
    _src[reserved_kernel_index].pfn = 0;
    _src[reserved_kernel_index].valid = 0;
    _src[reserved_kernel_index].prot = 0;

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    return kc_in;
}

/**
 * @brief 
 * 
 * @param kc_in 
 * @param pcb1 
 * @param pcb2 
 * @return KernelContext* 
 */
KernelContext *KCSwitch(KernelContext *kc_in, void *pcb1, void *pcb2) {

    pcb_t *curr = (pcb_t *) pcb1;
    pcb_t *next = (pcb_t *) pcb2;

    TracePrintf(0,"KCSwitch called! curr process id: %d\n",(int)(curr->pid));

    memcpy(&(curr->kernel_context), kc_in, sizeof(KernelContext));
    pte_t *k_pt = (pte_t *) ReadRegister(REG_PTBR0);

    // iterate through next_pcb's kernel stack and update region0 pagetable
    int stack_base = ( (int) KERNEL_STACK_BASE >> PAGESHIFT );


    // change the pte to point to the same stack framme
    for (int i = 0 ; i < KERNEL_STACK_SIZE; i++ ) {
        k_pt[stack_base + i].pfn = next->kernel_stack_pt[i].pfn;
        k_pt[stack_base + i].prot = next->kernel_stack_pt[i].prot;
        k_pt[stack_base + i].valid = next->kernel_stack_pt[i].valid;
    }

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
    // WriteRegister(REG_PTBR0, (unsigned int) k_pt);
    TracePrintf(0,"done with KCSwitch, returning next_pcb kernel context\n");
    // return pointer to KernelContext in new PCB

    return &(next->kernel_context);
}