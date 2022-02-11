/*
 *  boot.c
 *  
 *  contains the KernelStart function, boots and intializes
 *  our OS and its first process
 */

#include <ykernel.h>
#include <hardware.h>
#include <ylib.h>
#include <yuser.h>
#include <yalnix.h>
//#include "interrupt.h"
#include "process.h"
#include "traphandlers.h"


//handler_func_t *InterruptVectorTable[TRAP_VECTOR_SIZE]; // the interrupt vector table is an array of interrupt handlers (type handler_t)


void SetRegion0_pt(pte_t *k_pt, int k_pt_size, int bit_vector[]);
void SetRegion1_pt(pte_t *u_pt, int u_pt_size, int bit_vector[],UserContext *uctxt);
void DoIdle(void);

enum {
    // default values
    DEFAULT_TRACE_LEVEL   =    1,
    MAX_TRACE_LEVEL       =   10, // yo what is the max trace level am I blind
    DEFAULT_TICK_INTERVAL =  400, // in ms !
    PAGE_FREE             =    1,
    PAGE_NOT_FREE         =    0,
    VALID_FRAME           =    1,
    INVALID_FRAME         =    0,
    VM_ENABLED            =    1,
    VM_DISABLED           =    0,
    ADDR_SPACE_ENTRY_SIZE =    4,


    // permissions for page table, in order X W R -- NOT R W X >:(
    
    
    X_NO_W_R              =    5,      // read allowed, no write, exec allowed
    NO_X_NO_W_NO_R        =    0,      // no read, no write, no execUserContext 
    X_W_R                 =    7,      // read allowed, write allowed, exec allowed
    NO_X_W_R              =    3,      // read allowed, write allowed, no exec
    NO_X_NO_W_R           =    1
};


    
// initialize pointer to bit vector (an array of integers of size num_of_frame)
int *ptr_bit_vector;


// kernel brk CHECK: if I change kernel_brk does _kernel_orig_brk change too
void *kernel_brk;

// =================================
//  yalnix-switch-configured values
// =================================
// TODO for all of these ^^: find out where we have to put these variables to cause actual change

// tracing levels for kernel, hardware, and user
int k_tracing_level = DEFAULT_TRACE_LEVEL;  // kernel tracing level
int h_tracing_level = DEFAULT_TRACE_LEVEL;  // hardware tracing level
int u_tracing_level = DEFAULT_TRACE_LEVEL;  // user tracing level

// tracefile that traceprint writes to
char* tracefile; //= TRACE;

// tick interval of clock
int tick_interval = DEFAULT_TICK_INTERVAL;



/*
 * KernelStart
 *
 * initializes our OS: page tables for region0 and region1
 * accepts configurations through cmdline switches, and starts our first
 * process
 *
 * takes in
 *  - cmd_args[], arguments from cmdline
 *  - pmem_size, size of physical memory for our OS, it's in BYTES
 *  - uctxt, usercontext to go into our idle process
 * 
 */
void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt) {    

    TracePrintf(0,"DEBUG: Entering KernelStart\n");

    // if kernel_brk hasn't been changed by SetKernelBrk

    // here we haven't called malloc yet, so we can set kernel_brk to orig_brk  
    kernel_brk = _kernel_orig_brk;
    
    
    // total number of frames in PM
    int num_of_frames = pmem_size / PAGESIZE;

    // set up bit vector (to keep track of free frames)
    int bit_vector[num_of_frames];
    // global pointer point to bit_vector for other functions access
    ptr_bit_vector = bit_vector;
    // initialize all frames to free
    for (int fr_number = 0; fr_number < num_of_frames; fr_number++) {
        bit_vector[fr_number] = PAGE_FREE;
    }

    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */
    InterruptVectorTable = malloc(sizeof(handler_func_t) * TRAP_VECTOR_SIZE);
    *(InterruptVectorTable + TRAP_KERNEL) = TrapKernelHandler;
    *(InterruptVectorTable + TRAP_CLOCK) = TrapClockHandler;
    *(InterruptVectorTable + TRAP_ILLEGAL) = TrapIllegalHandler;
    *(InterruptVectorTable + TRAP_MEMORY) = TrapMemoryHandler;
    *(InterruptVectorTable + TRAP_MATH) = TrapMathHandler;
    *(InterruptVectorTable + TRAP_TTY_RECEIVE) = TrapTTYReceiveHandler;
    *(InterruptVectorTable + TRAP_TTY_TRANSMIT) = TrapTTYTransmitHandler;
    *(InterruptVectorTable + TRAP_DISK) = TrapDiskHandler;
    for (int i = 8;i <= TRAP_VECTOR_SIZE;i++) {
        *(InterruptVectorTable + i) = NULL;
    }

    //update register on location of ivt
    WriteRegister(REG_VECTOR_BASE, (int)InterruptVectorTable); 

    TracePrintf(0,"Interrupt vector table is at: %x\n",InterruptVectorTable);
    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */

// ================================= //
//   INITIALIZE REGION0 PAGE TABLE   //
// ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region0 page table\n");

    // number of kernel pagetable entires
    int k_pt_size = VMEM_0_SIZE / PAGESIZE;
    TracePrintf(0,"\t~~~k_pt_size = %d~~~\n",k_pt_size);

    // declare kernel page table
    pte_t *k_pt =  malloc(sizeof(pte_t) * k_pt_size);
    if (k_pt == NULL) {
        TracePrintf(0,"ERROR, malloc failed for kernel page table\n");
    }

    SetRegion0_pt(k_pt,k_pt_size,bit_vector);

// ================================= //
//   INITIALIZE REGION1 PAGE TABLE   //
// ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region 1 page table, just stack though\n");
    // number of user pagetable entires
    int u_pt_size = VMEM_1_SIZE / PAGESIZE;

    // MAX_PT_LEN is a constant in hardware.h, the max #of pagetable entries
    if (u_pt_size < MAX_PT_LEN) {
        TracePrintf(0,"Something went wrong, we have too many page table entries (we have %d, max is %d)", k_pt_size,MAX_PT_LEN);
    }

    // define user page table, a pointer to the first page table entry
    pte_t *u_pt = malloc(sizeof(pte_t) * u_pt_size);
    if (u_pt == NULL) {
        TracePrintf(0,"ERROR, malloc failed for user page table\n");
    }

    SetRegion1_pt(u_pt,u_pt_size,bit_vector,uctxt);

    TracePrintf(0,"DEBUG: done with region1 page table\n");

    // TracePrintf(0,"Going to malloc a whole lot...\n");
    // int *e = malloc(90000);

// enable VM
    TracePrintf(0,"DEBUG: Enabling virtual memory\n");
    WriteRegister(REG_VM_ENABLE,VM_ENABLED);

// ============================= //
//   SET UP IDLEPCB AND DOIDLE   //
// ============================= //
    pcb_t *idlePCB = init_process();
    if (idlePCB == NULL) {
        // DO SOME ERROR HANDLING
    }
    idlePCB->user_context = uctxt;
    idlePCB->user_context->pc = DoIdle;
    idlePCB->user_context->sp = (void *) u_pt;
    // idlePCB's stack has been set while setting up region1's page table
    TracePrintf(0,"sp = %x, pc = %x\n",idlePCB->user_context->sp,idlePCB->user_context->pc);

    void* addr = (void*) DOWN_TO_PAGE(idlePCB->user_context->pc);

    int vpn = ((int)addr >> PAGESHIFT) - (VMEM_0_BASE >> PAGESHIFT);
    TracePrintf(0,"pc is at vpn %x\n",vpn);

    pte_t temp = *(k_pt + vpn);
    TracePrintf(0,"valid bit %d, pfn %x\n",temp.valid,temp.pfn);


    idlePCB->kernel_context = NULL;
    idlePCB->pid = helper_new_pid((void *) ReadRegister(REG_PTBR1));
    idlePCB->kernel_page_table = k_pt;
    idlePCB->user_page_table = u_pt;
    /* =================== idle =================== */
    TracePrintf(0, "Debug: Initializing Idle Proccess\n");

    TracePrintf(0,"Exiting KernelStart...\n");  
}



// pcb_t *boot_process = init_process();

    // boot_process->pid = 0; // find this value somewhere
    // boot_process->parent = NULL;

    // boot_process->user_context = uctxt;
    // boot_process->kernel_context = NULL; // null for now. INIT here?

    // int kernal_page_table_size = VMEM_0_SIZE / PAGESIZE;
    // int userland_page_table_size = VMEM_1_SIZE / PAGESIZE;

    // pte_t *region0_pagetable[kernal_page_table_size]; //READREGISTER FUNCTION INSTEAD?
    // pte_t *region1_pagetable[userland_page_table_size];

    // boot_process->user_page_table = region1_pagetable;
    // boot_process->kernal_page_table = region0_pagetable;

    //=== leave user alone for now. but can go here ===//

    
    // interate through each page table entry to index's
    // for (int i = 0; i < kernal_page_table_size; i++) {

    //     //void *lower_addr = VMEM_0_BASE;
    //     //void *lower_addr = 

    //     if (i = 0) {
    //         boot_process->user_text_pt_index = 0; // because this is the bottom;
    //     }

    //     if 
    // }

void DoIdle(void) {
    while(1) {
        TracePrintf(1,"DoIdle\n");
        Pause();
    }
}

void SetRegion0_pt(pte_t *k_pt, int k_pt_size, int bit_vector[]) {
    // tell hardware where Region0's page table, (virtual memory base address of page_table)
    //WriteRegister(REG_PTBR0, &kernal_page_table);
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


    int pt_index; // page table index
    // for each each page table entry
    for (pt_index = 0; pt_index < k_pt_size; pt_index++) {
        TracePrintf(0,"\t\tat address %x\n",pt_index << PAGESHIFT);
    // .text
        // if page table index is between .text's virtual page number range
        if ((pt_index >= text_vp_lowest - vp0) && (pt_index < text_vp_highest - vp0)) { // FYI changed from kernal_data_start to _kernal_data_start

            // create pte with .text permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = X_NO_W_R; // we can read and execute our code 
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;
            
            TracePrintf(0,"~~~.text: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,text_vp_lowest,text_vp_highest,entry.pfn);


            // update bitvector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }
        
    // .data
        // if page table index is between .data's virtual page number range
        else if ((pt_index >= data_vp_lowest - vp0) && (pt_index < data_vp_highest - vp0)) {
            
            // create pte with .data permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read and execute only
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            TracePrintf(0,"~~~.data: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,data_vp_lowest,data_vp_highest,entry.pfn);


            // update bitvector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }

    // heap
        // if page table index is between heap's virtual page number range
        else if ((pt_index >= heap_vp_lowest - vp0) && (pt_index < heap_vp_highest - vp0)) {



            // create pte with .heap permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read, write but not execute our heap
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            TracePrintf(0,"~~~heap: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,heap_vp_lowest,heap_vp_highest,entry.pfn);

            // update bit vector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }
    
    // stack
        // if page stable index is between the stack's virtual page number range
        else if ((pt_index >= stack_vp_lowest - vp0) && (pt_index < stack_vp_highest - vp0)) { // casting void * to int here. Could be an issues, maybe

            // create pte with stack permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R; // we can read, write but not execute our stack
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            TracePrintf(0,"~~~stack: pt_index = %d,low = %d, high = %d => pfn = %d~~~\n",pt_index,stack_vp_lowest,stack_vp_highest,entry.pfn);

            // update bit vector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }

        // otherwise
        else {

            //  create an invalid page entry
            pte_t entry;
            entry.valid = INVALID_FRAME;
            *(k_pt + pt_index - vp0) = entry;

            // update bit vector
            bit_vector[pt_index] = PAGE_FREE;
        }

    }
    
    TracePrintf(0,"DEBUG: Done initializing region0 page table\n");

    // DELETE ME =====================
    TracePrintf(0,"\nPrinting stuff inside kernel page table...\n");
    int index = vp0;
    for (index; index < VMEM_0_LIMIT >> PAGESHIFT; index++) {
        pte_t temp = *(k_pt + index - vp0);
        TracePrintf(0,"index: %d, pfn: %x\n",index - vp0,temp.pfn);
    }

    // ================================
}

void SetRegion1_pt(pte_t *u_pt, int u_pt_size, int bit_vector[],UserContext *uctxt) {


    TracePrintf(0,"Entering SetRegion1_pt\n");

    TracePrintf(0,"VMEM_1_LIMIT: %x\n",VMEM_1_LIMIT);

    // vp0 = virtual page number 0 of region1
    int vp0 = VMEM_1_BASE >> PAGESHIFT;
    // pf0 = physical frame number 0
    int pf0 = PMEM_BASE >> PAGESHIFT;

    // tell hardware where Region1's page table, (virtual memory base address of page_table)
    WriteRegister(REG_PTBR1,(unsigned int)u_pt); 

    // tell hardware the number of pages in Region1's page table
    WriteRegister(REG_PTLR1,u_pt_size);

    // when we set up region1 we must do stack only, one valid page for it

    // find the first free frame in region1 memory space, so we start
    // indexing after all of kernel's frame indices

    // have pt_index start at vp0 for region1

    // stack is 1 address space entry below the top of region1's address space
    uctxt->sp = (void*)VMEM_1_LIMIT - ADDR_SPACE_ENTRY_SIZE;

    TracePrintf(0,"Set stack pointer to %x\n",uctxt->sp);

    void *bottom_of_stack_page = (void *)DOWN_TO_PAGE(uctxt->sp);

    // now we want to set up an entry in the region1 page table
    // this is the index of the page table
    int vpn = (int)bottom_of_stack_page >> PAGESHIFT; // equals vp0 for region1 pagetable
    // so here vpn = pfn

    int u_pt_index = vpn - vp0;

    TracePrintf(0,"page table index %x, which is at vpn %x\n",u_pt_index,vpn);

    TracePrintf(0,"pf0 is %x\n",pf0);

    // create pte with stack permissions
    pte_t entry;
    entry.valid = VALID_FRAME;
    entry.prot = NO_X_W_R; // we can read, write but not execute our stack
    entry.pfn = u_pt_index + vp0+ pf0;
    *(u_pt + u_pt_index) = entry;

    TracePrintf(0,"~~~user stack: found free frame at user_pt_index = %x => pfn = %x~~~\n",u_pt_index,entry.pfn);

    // update bit vector
    bit_vector[u_pt_index] = PAGE_NOT_FREE;
    

    // DELETE ME =====================
    TracePrintf(0,"\nPrinting stuff inside user page table...\n");
    int index = vp0;
    for (index; index < VMEM_1_LIMIT >> PAGESHIFT; index++) {
        pte_t temp = *(u_pt + index - vp0);
        TracePrintf(0,"index: %d, pfn: %x\n",index - vp0,temp.pfn);
    }

    TracePrintf(0,"Index of stack pointer is %x\n", (int)uctxt->sp >> PAGESHIFT );
    // ================================
    
    TracePrintf(0,"DEBUG: Done initializing region1 page table\n"); 

}

/*
 * SetKernelBrk
 *
 * assume that kernel_brk is correct when vm is enabled
 * assuming that there's still a 1 to 1 correspondence between kernel virtual memory
 * and kernel physical memory, so kernel heap must be contiguous
 * 
 * reminder that brk points to the first invalid address, so for us, the address
 * of the first invalid frame 
 * 
 *  takes in
 *      - addr: proposed address of new brk
 *  returns
 *      - 0 if success
 *      - -1 if fail
 */
int SetKernelBrk(void* addr) {

    TracePrintf(0,"DEBUG: Entering SetKernelBrk\n");
    // check if VM enabled
    int vm_enabled = ReadRegister(REG_VM_ENABLE);

    // vp0 = first virtual page number of region0
    int vp0 = VMEM_0_BASE >> PAGESHIFT;
    // pf0 = first frame number for physical memory
    int pf0 = PMEM_BASE >> PAGESHIFT;

    TracePrintf(0,"DEBUG: checking if vm enabled...\n");

    // if so
    if (vm_enabled == VM_ENABLED) {

        TracePrintf(0,"DEBUG: vm is enabled...\n");

        // check if given address is valid, if invalid, give ERROR
        if ((kernel_brk == NULL) || (kernel_brk > (void *)KERNEL_STACK_BASE)) {
            TracePrintf(0,"ERROR, we got an invalid brk, it's either NULL or within the kernel stack");
            return ERROR;
        }
    
        // index in page table of kernel brk
        int index = (int)kernel_brk >> PAGESHIFT - vp0;

        // index in page table of address given
        int addr_index = (int)addr >> PAGESHIFT - vp0;

        if (addr_index <= index) {
            TracePrintf(0,"ERROR, given address is less than or equal to current brk");
            return ERROR;
        }

        // check if virtual addresses between current brk and given address are taken
        for (index ; index < addr_index; index++) {
            if (*(ptr_bit_vector + index) == PAGE_NOT_FREE) {
                TracePrintf(0, "ERROR, at least 1 page between kernel brk and new brk has already been taken\n");
                return ERROR;
            }
        }
        
        // get user page table
        pte_t *u_pt = (pte_t *)ReadRegister(REG_PTBR0);

        index = (int)kernel_brk >> PAGESHIFT - vp0;
        // if all good, take up the addresses between current brk and address provided
        for (index ; index < addr_index ; index++) {

            // update bit_vector
            *(ptr_bit_vector + index) = PAGE_NOT_FREE;

            // update page table
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = NO_X_W_R;
            entry.pfn = index + pf0;

            *(u_pt + index - vp0) = entry;
        }

        // update brk
        kernel_brk = (void *)DOWN_TO_PAGE(addr);

        TracePrintf(0,"DEBUG: Exiting SetKernelBrk\n");
        return 0;
    } 
    
    // if not
    else if (vm_enabled == VM_DISABLED) {
        
        TracePrintf(0,"DEBUG VM not enabled yet...\n");
    // get how far beyond kernel-orig_brk we are

        // index in page table of original brk
        int index = (int)_kernel_orig_brk >> PAGESHIFT- vp0;

        int num_pages_above_orig_brk = 0;

        // while the bit vector at index is taken
        while (*(ptr_bit_vector + index) == PAGE_NOT_FREE) {
            
            // inc count
            num_pages_above_orig_brk++;
            // inc index
            index++;

            // if we reach stack, ERROR
            if (index * PAGESIZE + VMEM_0_BASE >= KERNEL_STACK_BASE) {
                TracePrintf(1,"ERROR, we've hit stack smh\n");
                return ERROR;
            }
        }

        // kernel_brk is the first address of the first free page        
        kernel_brk = (void *)((index + vp0) << PAGESHIFT);

        TracePrintf(0,"DEBUG: detected kernel_brk to be at %x\n",kernel_brk);
        
        // traceprint count to show how far beyond kernel_orig_brk is
        TracePrintf(0,"We are %d pages above kernel_orig_brk!\n",num_pages_above_orig_brk);
    
        TracePrintf(0,"DEBUG: Exiting SetKernelBrk\n");
        return 0;
    }

    // return ERROR if we somehow get here
    else {
        TracePrintf(1,"ERROR, VM_ENABLED wasn't 0 or 1\n");
        return ERROR;
    }

    
        
        
        

       

    
        
    
    
}
