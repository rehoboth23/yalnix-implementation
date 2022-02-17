/*
 *  contextswitch.c
 *  
 *  holds the functions KCCopy and KCSwitch
*/


#include <hardware.h>
#include <ykernel.h>
#include "process.h"

extern pte_t *k_pt;
extern int *bit_vector;

enum {
    // default values
    PAGE_FREE             =    1,
    PAGE_NOT_FREE         =    0,
    VALID_FRAME           =    1,
    INVALID_FRAME         =    0,

    // permissions for page table, in order X W R -- NOT R W X >:(
    
    
    X_NO_W_R              =    5,      // read allowed, no write, exec allowed
    NO_X_NO_W_NO_R        =    0,      // no read, no write, no execUserContext 
    X_W_R                 =    7,      // read allowed, write allowed, exec allowed
    NO_X_W_R              =    3,      // read allowed, write allowed, no exec
    NO_X_NO_W_R           =    1
};

KernelContext *KCCopy(KernelContext *kctxt, pcb_t *new_pcb_ptr, void *not_used) {

    if (kctxt == NULL || new_pcb_ptr == NULL) {
        TracePrintf(0,"KCCopy provided NULL arguments!\n");
        return NULL;
    }

    // copy kernel context
    KernelContext *new_kctxt = malloc(sizeof(KernelContext));
    if (new_kctxt == NULL){
        TracePrintf(0,"in KCCopy, malloc failed\n");
    }
    memcpy(new_kctxt,kctxt,sizeof(KernelContext));


// allocate some dummy pages
    // start looking from top of kernel stack downwards
    int count = 0;
    for (int i = (VMEM_1_BASE >> PAGESHIFT) - 1; i > VMEM_0_BASE >> PAGESHIFT; i--) {
        
        // if there's a free frame
        if (bit_vector[i] == PAGE_FREE) {
            TracePrintf(0,"Found a free frame for dummy page at %d\n",i);

            // mark as not free, and save the physical frame number
            bit_vector[i] = PAGE_NOT_FREE;


            TracePrintf(0,"KCCopy dummy page: vpn %d points to pfn %d\n",i,new_pcb_ptr->kernel_stack_frames[count]);
            // set up dummy page table entry
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R;
            entry.pfn = new_pcb_ptr->kernel_stack_frames[count];

            k_pt[i] = entry;
            // 
            
            // copy contents of stack page to dummy page
            TracePrintf(0,"Copying from addr %x to %x...\n",KERNEL_STACK_LIMIT - (count+1) * PAGESIZE,i << PAGESHIFT);
            memcpy((void*)(i << PAGESHIFT),(void *)(KERNEL_STACK_LIMIT - (count+1) * PAGESIZE),PAGESIZE);

            // inc count, which indexes new pcb's kernel stack frame
            count++;

            TracePrintf(0,"new process's page table at vpn %d points to pfn %d\n",(KERNEL_STACK_LIMIT-count * PAGESIZE) >> PAGESHIFT,entry.pfn);
            // create new process's page table, make kernel stack region point to our specific page frame
            new_pcb_ptr->kernel_page_table[(KERNEL_STACK_LIMIT - count * PAGESIZE) >> PAGESHIFT] = entry;
            
            // flush stack in case
            WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_KSTACK);

            // flush region1 page table, we copied it earlier
            WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);
            
            //deactivate dummy page
            TracePrintf(0,"Deactivating dummy page...\n");
            k_pt[i].valid = 0;
        }
        // if we've allocated enough stack frames
        if (count >= KERNEL_STACK_MAXSIZE/PAGESIZE) {
            break;
        }
    }


    return kctxt;
}

KernelContext *KCSwitch(KernelContext *kc_in,void *curr_pcb_p, void *next_pcb_p) {

    TracePrintf(0,"KCSwitch called! curr process id: %d\n",(int)((pcb_t *)curr_pcb_p)->pid);

// copy kernel context to old pcb
    KernelContext tmp = *kc_in;
    ((pcb_t *)curr_pcb_p)->kernel_context = &tmp;

// change region 0 stack mappings to that for new PCB


    // iterate through next_pcb's kernel stack and update region0 pagetable
    int stack_index = (int)KERNEL_STACK_BASE >> PAGESHIFT;
    int counter = 0;

    // change the pte to point to the same stack framme
    for (stack_index ; stack_index < KERNEL_STACK_LIMIT >> PAGESHIFT; stack_index++ ) {

        TracePrintf(0,"in KCSwitch: changing kernel page table from %d to %d...\n",k_pt[stack_index].pfn, ((pcb_t *)next_pcb_p)->kernel_stack_frames[counter]);
        
        // page frame number of in page table for kernel stack is set to process B's frame
        (k_pt[stack_index]).pfn = ((pcb_t *)next_pcb_p)->kernel_stack_frames[counter];
        counter++;
    }
    
    TracePrintf(0,"done with KCSwitch, returning next_pcb kernel context\n");

    // return pointer to KernelContext in new PCB
    return ((pcb_t *)next_pcb_p)->kernel_context;
}


