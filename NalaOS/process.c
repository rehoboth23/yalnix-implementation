/*
 *  process.c
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#include <ylib.h>
#include <hardware.h>
#include <ykernel.h>
#include "process.h"
#include "kernel.h"
#include "include.h"

/**
 * @brief 
 * 
 * @param uctxt 
 * @return pcb_t* 
 */
pcb_t *init_process(UserContext *uctxt) {

    if (uctxt == NULL) {
        TracePrintf(0, "Error: User Context null in init_process\n");
    }

    pcb_t *process = malloc(sizeof(pcb_t));

    if (process == NULL) {
        TracePrintf(0, "Error: Init process failed.\n");
        return NULL;
    }

    // initialize all values to NULL or zero
    process->num_children = 0;
    process->clock_ticks = 0;
    process->blocked_code = NOT_BLOCKED;
    process->exit_code = 0;
    process->tty_terminal = 0;

    // user context and kernel context
    memset(&(process->user_context), 0, sizeof(UserContext));
    memset(&(process->kernel_context), 0, sizeof(KernelContext));
    memcpy(&(process->user_context), uctxt, sizeof(UserContext));

    // things to do with pid and parent pid
    process->pid = helper_new_pid(process->user_page_table);
    TracePrintf(0, "Allocated PID -> %d\n", process->pid);
    if (activePCB == NULL) {
        process->ppid = process->pid;
    }
    else {
        process->ppid = activePCB->pid;
        activePCB->num_children++;
    }

    // set up user page table
    for(int index = 0; index < USER_PT_SIZE; index++ ) {
        process->user_page_table[index].valid = INVALID_FRAME;
        process->user_page_table[index].pfn = 0;
        process->user_page_table[index].prot = NO_X_NO_W_NO_R;
    }
    // set up kernel stack
        // if we're looking at any other process
    if (process->pid != 0) {
        for(int index = 0; index < KERNEL_STACK_SIZE; index++ ) {
            int pfn = AllocatePFN();
            if (pfn == ERROR) {
                TracePrintf(0, "ERROR: Invalid PFN in init process.\n");
                free_addr_space(process->user_page_table, process->kernel_stack_pt);
                return NULL;
            }
            process->kernel_stack_pt[index].valid = INVALID_FRAME;
            process->kernel_stack_pt[index].pfn = pfn;
            process->kernel_stack_pt[index].prot = NO_X_W_R;
        }
    } else {    // first (kernel) process
        pte_t *k_pt = ( pte_t *) ReadRegister(REG_PTBR0);  // current kernel stack should be regioin 0 stack
        int kernel_stack_base = (int) KERNEL_STACK_BASE >> PAGESHIFT;
        for (int i = 0; i < KERNEL_STACK_SIZE; i++) {
            process->kernel_stack_pt[i].pfn = k_pt[kernel_stack_base + i].pfn;
            process->kernel_stack_pt[i].prot = NO_X_W_R;
            process->kernel_stack_pt[i].valid = VALID_FRAME;
        }
    }

    // initialize indices of main segments
    process->user_stack_pt_index = 0;
    process->user_heap_pt_index = 0;
    process->user_text_pt_index = 0;
    process->user_data_pt_index = 0;
    

    return process;
}

/**
 * @brief frees the address space within a given page table and kernel stack
 * 
 * @param u_pt 
 * @param k_stack 
 */
int free_addr_space(pte_t *u_pt, pte_t *k_stack) {

    if (u_pt == NULL || k_stack == NULL) {
        TracePrintf(0, "ERROR: free_addr_space, null pte_t pointer.\n");
        return ERROR;
    }

    // loops through kernel stack
    for (int i = 0; i < KERNEL_STACK_SIZE; i++) {
        if (k_stack[i].valid == VALID_FRAME) {
            if (DeallocatePFN(k_stack[i].pfn) == ERROR) {
                TracePrintf(0, "ERROR: free_addr_space, deallocation k_stack error.\n");
                return ERROR;
            }
        }
    }
    // loops through user pagetable
    for (int i = 0; i < USER_PT_SIZE; i++) {
        if (u_pt[i].valid == VALID_FRAME) {
            if (DeallocatePFN(u_pt[i].pfn) == ERROR) {
                TracePrintf(0, "ERROR: free_addr_space, deallocation u_pt error.\n");
                return ERROR;
            }
        }
    }

    return 0;
}

/**
 * @brief deletes the given process by freeing its page table and kernel stack
 * 
 * @param pcb 
 */
int delete_process(pcb_t *pcb) {
    if (pcb == NULL) {
        TracePrintf(0, "ERROR: delete_process, null pcb_t pointer.\n");
        return ERROR;
    }
    free_addr_space(pcb->user_page_table, pcb->kernel_stack_pt);
    free(pcb);
    return 0;
}

/**
 * @brief copies user page table
 * 
 * @param u_pt1 
 * @param u_pt2 
 * @param k_pt 
 * @param reserved_kernel_index 
 * @return int 
 */
int CopyUPT(pte_t *u_pt1, pte_t *u_pt2, pte_t *k_pt, int reserved_kernel_index) {

    if (u_pt1 == NULL || u_pt2 == NULL || k_pt == NULL) {
        TracePrintf(0, "ERROR: CopyUPT, null pte pointer.\n");
        return ERROR;
    }

    if (reserved_kernel_index < 0) {
        TracePrintf(0, "Error: CopyUPT, reserved_kernel_index < 0\n");
        return ERROR;
    }

    // region 1 base
    int r1_base = (int) VMEM_1_BASE >> PAGESHIFT;

    // for each page in region1
    for (int i = 0; i < USER_PT_SIZE; i++) {    // allocate and copy user page table

        // if it's valid
        if (u_pt1[i].valid == VALID_FRAME) {
            // allocate some pfn for it
            int pfn = AllocatePFN();
            if (pfn == -1) {
                TracePrintf(0, "ERROR: Invalid PFN.\n");
                return ERROR;
            }
            
            // set up user page table, we give it a temporary prot for now
            // it'll be corrected in a few lines
            k_pt[reserved_kernel_index].pfn = pfn;
            u_pt2[i].pfn = pfn;
            u_pt2[i].prot = NO_X_W_R;
            u_pt2[i].valid = VALID_FRAME;

            // first copy to reserved kernel page
            void *dst = (void *) (reserved_kernel_index << PAGESHIFT);
            void *src = (void *) ( (r1_base + i) << PAGESHIFT );

            // flush the specific addresses
            WriteRegister(REG_TLB_FLUSH, (unsigned int) dst);
            // but we don't flush src
            // WriteRegister(REG_TLB_FLUSH, (unsigned int) src);
            
            // copy page at src into dst
            memcpy(dst, src, PAGESIZE);

            // inherit the correct protection
            u_pt2[i].prot = u_pt1[i].prot;
        }
    }
    return 0;
}

/**
 * @brief moves the current running process into the queue passed in
 * and then makes the next process in the ready queue the active process.
 * 
 * If there's nothing to run, we run idle.
 * 
 * @param moveActive <-- what is this? the current active process?
 */
int SwapProcess(queue_t *moveActive, UserContext *uctxt) {

    if (uctxt == NULL) {
        TracePrintf(0, "ERROR: SwapProcess, null usercontext pointer.\n");
        return ERROR;
    }

    queue_t *move;

    // if the current process is the idle process, then we don't change process
    if (activePCB->pid == idlePCB->pid) {
        move = NULL;
    }
    // otherwise,
    else {
        move = moveActive;
    }

    // if (active pcb is not idle or there is something in the ready) queue and the user context is valid
    if (!(activePCB->pid == idlePCB->pid &&  ready_q->size == 0)) {

        // get next ready process from queue
        pcb_t *next = queue_pop(ready_q);

        // if no ready process, idle
        if (next == NULL) next = idlePCB;
        pcb_t *tmp = activePCB;

        // update sp and pc
        activePCB->user_context.sp = uctxt->sp;
        activePCB->user_context.pc = uctxt->pc;

        // flush tlb
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
        // update ptbr1
        WriteRegister(REG_PTBR1, (unsigned int) next->user_page_table);
        // update the move queue?????????????
        if (move != NULL) {
            if (queue_add(move, activePCB, activePCB->pid) == ERROR) {
                TracePrintf(0, "ERROR: SwapProcess, null unable to add to queue.\n");
                return ERROR;
            }
        }
        activePCB = next;

        KernelContextSwitch(KCSwitch, tmp, next);

        // update sp and pc of uctxt for the new activePCB
        uctxt->sp = activePCB->user_context.sp;
        uctxt->pc = activePCB->user_context.pc;

        return 0;
    }
}