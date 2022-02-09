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
#include "interrupt.h"
#include "process.h"
#include "traphandlers.h"

/* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */
handler_func_t *InterruptVectorTable[TRAP_VECTOR_SIZE]; // the interrupt vector table is an array of interrupt handlers (type handler_t)

InterruptVectorTable[TRAP_KERNEL] = TrapKernelHandler;
InterruptVectorTable[TRAP_CLOCK] = TrapClockHandler;
InterruptVectorTable[TRAP_ILLEGAL] = TrapIllegalHandler;
InterruptVectorTable[TRAP_MEMORY] = TrapMemoryHandler;
InterruptVectorTable[TRAP_MATH] = TrapMathHandler;
InterruptVectorTable[TRAP_TTY_RECEIVE] = TrapTTYReceiveHandler;
InterruptVectorTable[TRAP_TTY_TRANSMIT] = TrapTTYTransmitHandler;
InterruptVectorTable[TRAP_DISK] = TrapDiskHandler;
/* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */
enum {
    // default values
    DEFAULT_TRACE_LEVEL   =    1,
    MAX_TRACE_LEVEL       =   10, // yo what is the max trace level am I blind
    DEFAULT_TICK_INTERVAL =  400, // in ms !
    PAGE_FREE             =    1,
    PAGE_NOT_FREE         =    0,
    VALID_FRAME           =    1,
    INVALID_FRAME         =    0,


    // permissions for page table
    R_NO_W_X              =    5,      // read allowed, no write, exec allowed
    NO_R_NO_W_NO_X        =    0,      // no read, no write, no exec
    R_W_X                 =    7,      // read allowed, write allowed, exec allowed
    R_W_NO_X              =    6      // read allowed, write allowed, no exec
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

    // update register on location of ivt
    WriteRegister(REG_VECTOR_BASE, InterruptVectorTable);

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


// ====================================== //
//   HANDLING SWITCHES IN CMDLINE INPUT   //
// ====================================== //

    int index = 0;

    // loop through cmd_args until null
    while (cmd_args[index] != NULL) {
    // == common yalnix options: == //

        // -x
        if (strcmp(cmd_args[index],"-x") == 0) {
            TracePrintf(0,"-x switch detected, but not implemented!\n");
            // use the X window system support for terminals
            // attached to Yalnix

            // we don't do until checkpoint 5
            
        }

        //-lk 
        else if (strcmp(cmd_args[index],"-lk") == 0) {
            TracePrintf(0,"-lk switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {
                
                // check if level is digit
                size_t length = sizeof(cmd_args[index]) - 1;
                int i;
                for (i = 0; i<length; i++) {
                    if (!isdigit(cmd_args[index][i])) {
                        TracePrintf(0,"Error, invalid tracing level, we don't accept %c\n",  cmd_args[index][i]);
                    }
                }

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the kernel
                    k_tracing_level = level;

                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n", level);
                
            } else TracePrintf(0,"Error, expected level after -lk switch\n");
        }

        //-lh  
        else if (strcmp(cmd_args[index],"-lh") == 0) {
            TracePrintf(0,"-lh switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {
                
                // check if level is digit
                size_t length = sizeof(cmd_args[index]) - 1;
                int i;
                for (i = 0; i<length; i++) {
                    if (!isdigit(cmd_args[index][i])) {
                        TracePrintf(0,"Error, invalid tracing level, we don't accept %c\n", cmd_args[index][i]);
                    }
                }

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the hardware
                    h_tracing_level = level;


                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n", level);
                
            } else TracePrintf(0,"Error, expected level after -lh switch\n");
        }

        //-lu 
        else if (strcmp(cmd_args[index],"-lu") == 0) {
            TracePrintf(0,"-lu switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {
                
                // check if level is digit
                size_t length = sizeof(cmd_args[index]) - 1;
                int i;
                for (i = 0; i<length; i++) {
                    if (!isdigit(cmd_args[index][i])) {
                        TracePrintf(0,"Error, invalid tracing level, we don't accept %c\n",  cmd_args[index][i]);
                    }
                }

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the user
                    u_tracing_level = level;


                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n", level);
                
            } else TracePrintf(0,"Error, expected level after -lu switch\n");
        }

        //-W TODO: CHECK IF DONE CORRECTLY!! office hours or ask prof
        else if (strcmp(cmd_args[index],"-W") == 0) {
            TracePrintf(0,"-W switch detected\n");

            // dump core if hardware helper encounters issue
            helper_maybort("TracePrintf");
            // TODO: CHECK IF THIS ^^ IS CORRECT
        }

    // == esoteric yalnix options == //

        // -t tracefile
        else if (strcmp(cmd_args[index],"-t") == 0) {
            TracePrintf(0,"-t switch detected\n");

            // get tracefile by incrementing index
            index++;

            // check if next input is valid
            if (cmd_args[index] != NULL) {
                
                // send traceprints to tracefile instead of TRACE
                tracefile = cmd_args[index];

            } else TracePrintf(0,"Error, expected traceprint after -t switch\n");
        }

        // -C NNN
        else if (strcmp(cmd_args[index],"-C") == 0) {
            TracePrintf(0,"-C switch detected\n");

            // get tick interval by incrementing index
            index++;

            if (cmd_args[index] != NULL) {

                int new_tick_interval = atoi(cmd_args[index]);

                // make sure tick interval is in range
                if (new_tick_interval <= 0) {

                    // if valid, set new clock tick interval
                    tick_interval = new_tick_interval;


                } else TracePrintf(0,"Error, invalid tick interval, we don't accept %d\n", new_tick_interval);
                
            } else TracePrintf(0,"Error, expected tick interval after -C switch\n");


        }
        
        // -In file 
            // check if first char of arg is -I
            // check that integer next to it is valid
            // check existence of file

            // if all good, feed terminal n with data from file
            // as if it were typed there
        
        // -On file 
            // check if first char of arg is -O
            // check that integer next to it is valid
            // check existence of file

            // if all good, feed output of terminal n to 
            // filename TTYLOG.n, or change that to file

        // or if there is a valid string, then it's the initial process
            // I think we only do ^ this condition ^ for index == 0

        // after all that, increment index
        index++;
    }
    
    // if there were no arguments, look for an executable called init and run that
    if (index == 0) {
        // TODO: run the executable called init

        // question: can we use fork and exec to run init
    }

    if (pmem_size <= 0) {
        TracePrintf(0,"Error! inputted pmem_size (%d) is negative or 0!\n", pmem_size);
    }

    

// ================================= //
//   INITIALIZE REGION0 PAGE TABLE   //
// ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region0 page table\n");

    // number of kernel pagetable entires
    int k_pt_size = VMEM_0_SIZE / PAGESIZE;

    // declare kernel page table
    pte_t *k_pt =  malloc(sizeof(pte_t) * k_pt_size);
    if (k_pt == NULL) {
        TracePrintf(0,"Error, malloc failed for kernel page table\n");
    }
    
    // tell hardware where Region0's page table, (virtual memory base address of page_table)
    //WriteRegister(REG_PTBR0, &kernal_page_table);
    WriteRegister(REG_PTBR0, (unsigned)k_pt);

    // tell hardware the number of pages in Region0's page table
    WriteRegister(REG_PTLR0,k_pt_size);

    
    // virtual page number of 0th page for region0
    int vp0 = VMEM_0_BASE >> PAGESHIFT;
    
    // physical page number of 0th frame
    int pf0 = PMEM_BASE >> PAGESHIFT;

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


    int pt_index; // page table index

    // for each each page table entry
    for (pt_index = 0; pt_index < k_pt_size; pt_index++) {
        
    // .text
        // if page table index is between .text's virtual page number range
        if ((pt_index >= text_vp_lowest - vp0) && (pt_index < text_vp_highest - vp0)) { // FYI changed from kernal_data_start to _kernal_data_start

            // create pte with .text permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_NO_W_X; // we can read and execute our code 
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            // update bitvector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }
        
    // .data
        // if page table index is between .data's virtual page number range
        else if ((pt_index >= data_vp_lowest - vp0) && (pt_index < data_vp_highest - vp0)) {

            // create pte with .data permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our globals
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            // update bitvector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }

    // heap
        // if page table index is between heap's virtual page number range
        else if ((pt_index >= heap_vp_lowest - vp0) && (pt_index < heap_vp_highest - vp0)) {
            
            // create pte with .heap permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our heap
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

            // update bit vector
            bit_vector[pt_index] = PAGE_NOT_FREE;
        }
    
    // stack
        // if page stable index is between the stack's virtual page number range
        else if ((pt_index >= stack_vp_lowest - vp0) && (pt_index < stack_vp_highest - vp0)) { // casting void * to int here. Could be an issues, maybe

            // create pte with stack permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our stack
            entry.pfn = pt_index - vp0 + pf0;
            *(k_pt + pt_index - vp0) = entry;

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

// ================================= //
//   INITIALIZE REGION1 PAGE TABLE   //
// ================================= //

    // number of user pagetable entires
    int u_pt_size = VMEM_1_SIZE / PAGESIZE;

    // vp0 = virtual page number 0 of region1
    vp0 = VMEM_1_BASE >> PAGESHIFT;
    // pf0 = pageframe0


    // MAX_PT_LEN is a constant in hardware.h, the max #of pagetable entries
    if (u_pt_size < MAX_PT_LEN) {
        TracePrintf(0,"Something went wrong, we have too many page table entries (we have %d, max is %d)", k_pt_size,MAX_PT_LEN);
    }

    // define user page table, a pointer to the first page table entry
    pte_t *u_pt = malloc(sizeof(pte_t) * u_pt_size);
    if (u_pt == NULL) {
        TracePrintf(0,"Error, malloc failed for user page table\n");
    }

    // tell hardware where Region1's page table, (virtual memory base address of page_table)
    WriteRegister(REG_PTBR1,(unsigned int)u_pt); 

    // tell hardware the number of pages in Region1's page table
    WriteRegister(REG_PTLR1,u_pt_size);

    // when we set up region1 we must do stack only, one valid page for it

    // find the first free frame in region1 memory space, so we start
    // indexing after all of kernel's frame indices

    // have pt_index start at vp0 for region1
    int u_pt_index = VMEM_1_BASE >> PAGESHIFT; // equals vp0 for region1 pagetable


    for (u_pt_index; u_pt_index< VMEM_1_LIMIT >> PAGESHIFT; u_pt_index++) {

        // if there is a free page
        if (bit_vector[u_pt_index] == PAGE_FREE) {

            // create pte with stack permissions
            pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our stack
            entry.pfn = u_pt_index + pf0;
            *(u_pt + (u_pt_index - vp0)) = entry;

            // SET STACK POINTER TO POINT AT THE RIGHT VIRTUAL ADDRESS
            // the right virtual address is (u_pt_index + vp0) << PAGESHIFT
            uctxt->sp = (void*)((u_pt_index + vp0) << PAGESHIFT);

            // update bit vector
            bit_vector[u_pt_index] = PAGE_NOT_FREE;
            break;
        }
    }
    TracePrintf(0,"DEBUG: Done initializing region1 page table\n");   
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

    // if so
    if (vm_enabled == 1) {

        // check if given address is valid, if invalid, give error
        if ((kernel_brk == NULL) || (kernel_brk > (void *)KERNEL_STACK_BASE)) {
            TracePrintf(0,"Error, we got an invalid brk, it's either NULL or within the kernel stack");
            return ERROR;
        }
    
        // index in page table of kernel brk
        int index = (int)kernel_brk >> PAGESHIFT - vp0;

        // index in page table of address given
        int addr_index = (int)addr >> PAGESHIFT - vp0;

        if (addr_index <= index) {
            TracePrintf(0,"Error, given address is less than or equal to current brk");
            return ERROR;
        }

        // check if virtual addresses between current brk and given address are taken
        for (index ; index < addr_index; index++) {
            if (*(ptr_bit_vector + index) == PAGE_NOT_FREE) {
                TracePrintf(0, "Error, at least 1 page between kernel brk and new brk has already been taken\n");
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
            entry.prot = R_W_NO_X;
            entry.pfn = index + pf0;

            *(u_pt + index - vp0) = entry;
        }

        // update brk
        kernel_brk = (void *)DOWN_TO_PAGE(addr);

        TracePrintf(0,"DEBUG: Exiting SetKernelBrk\n");
        return 0;
    } 
    
    // if not
    else if (vm_enabled == 0) {

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
                TracePrintf(1,"Error, we've hit stack smh\n");
                return ERROR;
            }
        }

        // kernel_brk is the first address of the first free page        
        kernel_brk = (void *)((index + vp0) << PAGESHIFT);
        
        // traceprint count to show how far beyond kernel_orig_brk is
        TracePrintf(1,"We are %d pages above kernel_orig_brk!\n",num_pages_above_orig_brk);
    
        // TODO: find out how this communicates with hardware
        TracePrintf(0,"DEBUG: Exiting SetKernelBrk\n");
        return 0;
    }

    // return error if we somehow get here
    else {
        TracePrintf(1,"Error, VM_ENABLED wasn't 0 or 1\n");
        return ERROR;
    }

    
        
        
        

       

    
        
    
    
}
