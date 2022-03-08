/*
 *  kernel.c
 *  
 *  contains the KernelStart function, boots and intializes
 *  our OS and its first process
 */

#include <yalnix.h>
#include <hardware.h>
#include <ykernel.h>
#include <ylib.h>
#include <yuser.h>
#include <load_info.h>
#include <fcntl.h>
#include <unistd.h>
#include "queue.h"
#include "pipe.h"
#include "kernel.h"
#include "include.h"
#include "traphandlers.h"
#include "process.h"



void *kernel_brk;
char* tracefile; //= TRACE;
pcb_t *activePCB;
int num_of_frames;
queue_t *ready_q;
queue_t *blocked_q;
queue_t *defunct_q;
list_t *pfn_list;
int global_clock_ticks;
pcb_t *idlePCB;
queue_t *ttyReadQueues[NUM_TERMINALS];
queue_t *ttyWriteQueues[NUM_TERMINALS];
char *ttyReadbuffers[NUM_TERMINALS];
int ttyWriteTrackers[NUM_TERMINALS];
int ttyReadTrackers[NUM_TERMINALS];
pipe_t *head_pipe;
list_t *lock_list;
list_t *cvar_list;
int lock_status[MAX_LOCKS];
int cvar_status[MAX_CVARS];
queue_t *lockAquireQueues[MAX_LOCKS];
queue_t *cvarWaitQueues[MAX_CVARS];

int id_tracker;


/**
 * @brief initializes our OS: page tables for region0 and region1
 * accepts configurations through cmdline switches, and starts our first
 * process
 * 
 * @param cmd_args arguments from cmdline
 * @param pmem_size pmem_size, size of physical memory for our OS, it's in BYTES
 * @param uctxt uctxt, usercontext to go into our idle process
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
    TracePrintf(0,"DEBUG: Entering KernelStart\n");

    int global_clock_ticks = 0;
    int id_tracker = 0;
    char *prog;

    if (cmd_args[0] == NULL) {
        prog = "progs/init";
    } else {
        prog = cmd_args[0];
    }

    // if kernel_brk hasn't been changed by SetKernelBrk

    // here we haven't called malloc yet, so we can set kernel_brk to orig_brk  
    kernel_brk = _kernel_orig_brk;
    
    // total number of frames in PM
    num_of_frames = pmem_size / PAGESIZE;

    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */
    InterruptVectorTable[TRAP_KERNEL] = TrapKernelHandler;
    InterruptVectorTable[TRAP_CLOCK] = TrapClockHandler;
    InterruptVectorTable[TRAP_ILLEGAL] = TrapIllegalHandler;
    InterruptVectorTable[TRAP_MEMORY] = TrapMemoryHandler;
    InterruptVectorTable[TRAP_MATH] = TrapMathHandler;
    InterruptVectorTable[TRAP_TTY_RECEIVE] = TrapTTYReceiveHandler;
    InterruptVectorTable[TRAP_TTY_TRANSMIT] = TrapTTYTransmitHandler;
    InterruptVectorTable[TRAP_DISK] = TrapDiskHandler;
    for (int i = 8;i <= TRAP_VECTOR_SIZE;i++) {
        InterruptVectorTable[i] = NULL;
    }

    //update register on location of ivt
    WriteRegister(REG_VECTOR_BASE, (int)InterruptVectorTable); 

    TracePrintf(0,"Interrupt vector table is at: %x\n",InterruptVectorTable);
    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */

// ================================= //
//   INITIALIZE REGION0 PAGE TABLE   //
// ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region0 page table\n");
    // declare kernel page table
    pte_t *k_pt = malloc(sizeof(pte_t) * KERNEL_PT_SIZE);
    if (k_pt == NULL) {
        TracePrintf(0,"ERROR, malloc failed for kernel page table\n");
    }

    // ================================= //
    //   INITIALIZE DUMMY REGION1 PAGE TABLE to enable Virtual Memory   //
    // ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region 1 page table, just stack though\n");
    // // define user page table, a pointer to the first page table entry
    pte_t *u_pt = malloc(sizeof(pte_t) * USER_PT_SIZE);
    if (u_pt == NULL) {
        TracePrintf(0,"ERROR, malloc failed for user page table\n");
    }

    if (SetRegion0_pt(k_pt, KERNEL_PT_SIZE) == ERROR) {
        TracePrintf(0,"ERROR, KernelStart, SetRegion0_pt failed.\n");
    }

    if (SetRegion1_pt(u_pt, USER_PT_SIZE) == ERROR) {
        TracePrintf(0,"ERROR, KernelStart, SetRegion0_pt failed.\n");
    }

    TracePrintf(0, "REG 0 = %x -> %x\n\t\tREG 1 = %x -> %x\n\t\t krenek_brk = %x\n", k_pt, k_pt +( KERNEL_PT_SIZE * sizeof(pte_t)),  u_pt, u_pt +( USER_PT_SIZE * sizeof(pte_t)), kernel_brk);
    // CLEAN UP REG0 BEFORE ENABLING VM (INCASE OF KERNEL_BRK HAS CHANGES)
    for(int i = (int) _kernel_data_end >> PAGESHIFT; i < (int) kernel_brk >> PAGESHIFT; i++) {
        k_pt[i].prot = NO_X_W_R;
        k_pt[i].pfn = i;
        k_pt[i].valid = VALID_FRAME;
    }

    // enable VM
    TracePrintf(0,"DEBUG: Enabling virtual memory\n");
    WriteRegister(REG_VM_ENABLE,VM_ENABLED);

    // set up all global structures
    if (SetUpGlobals() == ERROR) {
        TracePrintf(0,"ERROR: KernelStart, Set up globals failed.\n");
    }  


    TracePrintf(0,"=-=We've set up globals, head pipe is at %p\n",head_pipe);

// ============================= //
//   SET UP IDLEPCB AND DOIDLE   //
// ============================= //

    idlePCB = init_process(uctxt);

    if (idlePCB == NULL) {
        TracePrintf(0,"ERROR: KernelStart, idlePCB failed to init.\n");
    }

    int k_stack_base = KERNEL_STACK_BASE >> PAGESHIFT;

    if (LoadProgram("progs/idle", &(cmd_args[1]), idlePCB) == ERROR) { // idle args start from after kernel args
        TracePrintf(0,"ERROR: KernelStart, loadprogram for idle has failed.\n");
    } 

    activePCB = idlePCB;

    // FREE UP DUMMY REG1 PT
    free(u_pt);

    pcb_t *progPCB = init_process(uctxt);

    if (progPCB == NULL) {
        TracePrintf(0,"ERROR: KernelStart, progPCB failed to init.\n");
    }

    if (LoadProgram(prog, cmd_args, progPCB) == ERROR) {
        TracePrintf(0,"ERROR: KernelStart, loadprogram for prog has failed.\n");
    }
    
    if (queue_add(ready_q, progPCB, progPCB->pid) == ERROR) {
        TracePrintf(0,"ERROR: KernelStart, failed to add prog to ready q.\n");
    }

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
    WriteRegister(REG_PTBR1, (unsigned int) idlePCB->user_page_table);
    memcpy(uctxt, &(idlePCB->user_context), sizeof(UserContext));

    KernelContextSwitch(KCCopy, progPCB, NULL);

    TracePrintf(0,"Exiting KernelStart...\n");
    TracePrintf(0,"btw head pipe is at %p\n",head_pipe);
}

/**
 * @brief Set the Up Globals object
 * 
 */
int SetUpGlobals() {

    // global queues for processes
    ready_q = queue_init();
    blocked_q = queue_init();
    defunct_q = queue_init();

    // linked list of free page frames
    pfn_list = list_init();

    // global queues for processes reading/writing to terminal

    lock_list = list_init();
    cvar_list = list_init();
    for (int i = 0; i < NUM_TERMINALS; i++) {
        ttyReadQueues[i] = queue_init();
        ttyWriteQueues[i] = queue_init();
        ttyReadbuffers[i] = malloc(TERMINAL_MAX_LINE * sizeof(char));
        ttyWriteTrackers[i] = TERMINAL_OPEN;
        ttyReadTrackers[i] = 0;
        memset(ttyReadbuffers[i], 0, TERMINAL_MAX_LINE);
    }

    // error checking global queues

    for (int i = 0; i < MAX_LOCKS; i++) {
        lockAquireQueues[i] = queue_init();
        lock_status[i] = UNUSED_LOCK;
        if (list_add(lock_list, (void *) i) == ERROR) {
            TracePrintf(0, "ERROR: SetUpGlobals, adding to list failed");
            return ERROR;
        }
    }

    for (int i = 0; i < MAX_CVARS; i++) {
        cvarWaitQueues[i] = queue_init();
        cvar_status[i] = UNUSED_CVAR;
        list_add(cvar_list, (void *) MAX_LOCKS + i + 1);
    }



    if (ready_q == NULL || blocked_q == NULL || defunct_q == NULL || pfn_list == NULL) {
        TracePrintf(0, "ERROR: SetUpGlobals, initalization of queues failed\n");
        return ERROR;
    }

    // error checking pfn_list
    for (int fr_number = num_of_frames - 1; fr_number >= VMEM_1_BASE >> PAGESHIFT; fr_number--) {
        if (list_add(pfn_list, (void *) fr_number) == ERROR) {
            TracePrintf(0, "ERROR: SetUpGlobals, adding to list failed\n");
            return ERROR;
        }
    }

    // set up head pipe
    head_pipe = init_head_pipe();

    return 0;
}

/**
 * @brief function to create the VM for region0 pagetable
 * 
 * @param k_pt kernel page table
 * @param k_pt_size kernel page table size
 */
int SetRegion0_pt(pte_t *k_pt, int k_pt_size) {

    if (k_pt == NULL) {
        TracePrintf(0,"ERROR: SetRegion0_pt received a NULL argument\n");
        return ERROR;
    }
    
    // tell hardware where Region0's page table, (virtual memory base address of page_table)
    WriteRegister(REG_PTBR0, (unsigned)k_pt);

    // tell hardware the number of pages in Region0's page table
    WriteRegister(REG_PTLR0,k_pt_size);

    
    // virtual page number of 0th page for region0
    int vp0 = VMEM_0_BASE >> PAGESHIFT;
    TracePrintf(0,"\t~~~vp0 = %d~~~\n",vp0);
    
    // physical page number of 0th frame
    int pf0 = PMEM_BASE >> PAGESHIFT;
    TracePrintf(0,"\t~~~pf0 = %d~~~\n",pf0);

    // first valid .text virtual page number
    int text_vp_lowest = vp0;
    // first invalid virtual page number above .text
    int text_vp_highest = (int)_kernel_data_start >> PAGESHIFT;

    // lowest valid .data virtual page number
    int data_vp_lowest = text_vp_highest;
    // first invalid virtual page number above .data
    int data_vp_highest = (int)_kernel_data_end >> PAGESHIFT;

    // lowest valid heap virtual page number
    int heap_vp_lowest = data_vp_highest;
    // first invalid virtual page above heap
    int heap_vp_highest = (int)kernel_brk >> PAGESHIFT;

    // lowest valid stack virtual page number
    int stack_vp_lowest = (int)KERNEL_STACK_BASE >> PAGESHIFT;
    // first invalid virtual page above stack
    int stack_vp_highest = (int)KERNEL_STACK_LIMIT >> PAGESHIFT;

    TracePrintf(0,"DEBUG: values provided\n data start: %x\n data end: %x\n kernel brk: %x\n orig kernel brk: %x\n stack base: %x\n stack limit: %x\n",
    (int)_kernel_data_start,(int)_kernel_data_end,(int)kernel_brk,(int)_kernel_orig_brk,
    (int)KERNEL_STACK_BASE,(int)KERNEL_STACK_LIMIT);

    memset(k_pt, 0, sizeof(pte_t) * k_pt_size);

    int pt_index; // page table index
    // for each each page table entry
    for (pt_index = 0; pt_index < k_pt_size; pt_index++) {
        int pfn = pt_index + vp0;
    // .text
        // if page table index is between .text's virtual page number range
        if ((pt_index >= text_vp_lowest - vp0) && (pt_index < text_vp_highest - vp0)) { // FYI changed from kernel_data_start to _kernel_data_start

            // create pte with .text permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = X_NO_W_R; // we can read and execute our code 
            entry.pfn = pfn;
            k_pt[pt_index - vp0] = entry;
            
            TracePrintf(0,"~~~.text: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,text_vp_lowest,text_vp_highest,entry.pfn);

        }
        
    // .data
        // if page table index is between .data's virtual page number range
        else if ((pt_index >= data_vp_lowest - vp0) && (pt_index < data_vp_highest - vp0)) {
            
            // create pte with .data permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read and execute only
            entry.pfn = pfn;
            k_pt[pt_index - vp0] = entry;

            TracePrintf(0,"~~~.data: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,data_vp_lowest,data_vp_highest,entry.pfn);


        }

    // heap
        // if page table index is between heap's virtual page number range
        else if ((pt_index >= heap_vp_lowest - vp0) && (pt_index < heap_vp_highest - vp0)) {



            // create pte with .heap permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read, write but not execute our heap
            entry.pfn =  pfn;
            k_pt[pt_index - vp0] = entry;

            TracePrintf(0,"~~~heap: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,heap_vp_lowest,heap_vp_highest,entry.pfn);

        }
    
    // stack
        // if page stable index is between the stack's virtual page number range
        else if ((pt_index >= stack_vp_lowest - vp0) && (pt_index < stack_vp_highest - vp0)) { // casting void * to int here. Could be an issues, maybe

            // create pte with stack permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read, write but not execute our stack
            entry.pfn =  pfn;
            k_pt[pt_index - vp0] = entry;

            TracePrintf(0,"~~~stack: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,stack_vp_lowest,stack_vp_highest,entry.pfn);

        }

        // otherwise
        else {
            //  create an invalid page entry
            pte_t entry;
            entry.valid = INVALID_FRAME;
            entry.pfn =  pfn;
            entry.prot = 0;
            k_pt[pt_index - vp0] = entry;
        }

    }
    
    TracePrintf(0,"DEBUG: Done initializing region0 page table\n");
    return 0;
}

/**
 * @brief function to create the VM for region1 pagetable
 * 
 * @param u_pt user page table
 * @param u_pt_size user page table size
 * @param uctxt user context
 */
int SetRegion1_pt(pte_t *u_pt, int u_pt_size) {

    if (u_pt == NULL) {
        TracePrintf(0,"ERROR: SetRegion1_pt received a NULL argument\n");
        return ERROR;
    }

    TracePrintf(0,"DEBUG: Initializing region1 page table\n");

    // tell hardware where Region1's page table, (virtual memory base address of page_table)
    WriteRegister(REG_PTBR1,(unsigned int)u_pt); 

    // tell hardware the number of pages in Region1's page table
    WriteRegister(REG_PTLR1,u_pt_size);

    memset(u_pt, 0, sizeof(pte_t) * u_pt_size);

    TracePrintf(0,"DEBUG: Done initializing region1 page table\n"); 

    return 0;

}


/**
 * @brief assume that kernel_brk is correct when vm is enabled
 * assuming that there's still a 1 to 1 correspondence between kernel virtual memory
 * and kernel physical memory, so kernel heap must be contiguous
 * 
 * reminder that brk points to the first invalid address, so for us, the address
 * of the first invalid frame 
 * 
 * 
 * @param addr proposed address of new brk
 * @return int 
 *      - 0 if success
 *      - -1 if fail
 */
int SetKernelBrk(void* addr) {

    TracePrintf(0,"DEBUG: Entering SetKernelBrk ::: addr %x\n", addr);

    // check if given address is valid, if invalid, give ERROR
    if ((addr == NULL) || (addr >= (void *)KERNEL_STACK_BASE)) {
        TracePrintf(0,"ERROR, we got an invalid brk, it's either NULL or within the kernel stack");
        return ERROR;
    }

    // check if VM enabled
    int vm_enabled = ReadRegister(REG_VM_ENABLE);

    // vp0 = first virtual page number of region0
    int vp0 = VMEM_0_BASE >> PAGESHIFT;
    // pf0 = first frame number for physical memory
    int pf0 = PMEM_BASE >> PAGESHIFT;

    // get kernel page table
    pte_t *k_pt = (pte_t *) ReadRegister(REG_PTBR0);

    if (vm_enabled == VM_ENABLED) {

        TracePrintf(0,"DEBUG: vm is enabled...\n");

        // index in page table of kernel brk
        int index = (int)kernel_brk >> PAGESHIFT - vp0;

        // index in page table of address given
        int addr_index = (int)addr >> PAGESHIFT - vp0;

        if (addr_index > index) {   // ensure valid pages till address
            for (index ; index < addr_index; index++) {
                if (k_pt[index].valid == INVALID_FRAME) {
                    k_pt[index].valid = VALID_FRAME;
                    k_pt[index].prot = NO_X_W_R;
                }
            }
        }
        // update kernelbrk to the highest seen addr so far
        kernel_brk = (void *)DOWN_TO_PAGE(addr);
        TracePrintf(0,"DEBUG: Exiting SetKernelBrk\n");
        return 0;
    } else if (vm_enabled == VM_DISABLED) {
        if (addr > kernel_brk)  kernel_brk = (void *) DOWN_TO_PAGE(addr);
        TracePrintf(0,"DEBUG: detected kernel_brk to be at %x frame %d\n",kernel_brk, (int)kernel_brk >> PAGESHIFT- vp0);
        
        return 0;
    }

    // return ERROR if we somehow get here
    else {
        TracePrintf(1,"ERROR, VM_ENABLED wasn't 0 or 1\n");
        return ERROR;
    }

    return 0;
}

/**
 * @brief Get a Free PFN
 * 
 * @return int ERROR if not free pfn, pfn otherwise
 */
int AllocatePFN() {
    if (pfn_list->size == 0) return ERROR;
    int pfn = (int) list_pop(pfn_list);
    TracePrintf(0, "Allocating PFN -> %d\n", pfn);
    if (((void*)pfn == NULL) || (pfn == -1)) {
        TracePrintf(0,"AllocatePFN has failed\n");
        return ERROR;
    }
    return pfn;
}

/**
 * @brief 
 * 
 * @param pfn 
 */
int DeallocatePFN(int pfn) {
    TracePrintf(0, "Dellocating PFN -> %d\n", pfn);
    if (list_add(pfn_list, (void *) pfn) == -1) {
        TracePrintf(0,"DeallocatePFN has failed\n");
        return ERROR;
    }

    return 0;
}

/**
 * @brief 
 * 
 * @param pcb 
 * @return queue_t* 
 */
queue_t *CheckBlocked(pcb_t *pcb) {

    if (pcb == NULL) return NULL;

    switch (pcb->blocked_code) {
    case BLOCKED_DELAY:
        if (pcb->clock_ticks > 0) {
            pcb->clock_ticks--;
        } else {
            pcb->blocked_code = NOT_BLOCKED;
            return ready_q;
        }
        break;
    case BLOCKED_TTY_TRANSMIT:
        if (ttyWriteTrackers[pcb->tty_terminal] == TERMINAL_OPEN) {
            pcb->blocked_code = NOT_BLOCKED;
            pcb->tty_terminal = 0;
            return ready_q;
        }
        return NULL;
    default:
        break;
    }

    return blocked_q;
}
