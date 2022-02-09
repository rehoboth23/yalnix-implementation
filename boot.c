/*
 *  boot.c
 *  
 *  contains the KernelStart function, boots and intializes
 *  our OS and its first process
 */

#include "ykernel.h"
#include "hardware.h"
#include "ylib.h"
#include "yuser.h"
#include "interrupt.h"
#include "process.h"

handler_func_t InterruptVectorTable[TRAP_VECTOR_SIZE]; // the interrupt vector table is an array of interrupt handlers (type handler_t)

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


// =================================
//  yalnix-switch-configured values
// =================================
// TODO for all of these ^^: find out where we have to put these variables to cause actual change

// tracing levels for kernel, hardware, and user
int k_tracing_level = DEFAULT_TRACE_LEVEL;  // kernel tracing level
int h_tracing_level = DEFAULT_TRACE_LEVEL;  // hardware tracing level
int u_tracing_level = DEFAULT_TRACE_LEVEL;  // user tracing level

// tracefile that traceprint writes to
//char* tracefile = TRACE;

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

// ====================================== //
//   HANDLING SWITCHES IN CMDLINE INPUT   //
// ====================================== //

    int index = 0;

    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */
    InterruptVectorTable[TRAP_KERNEL] = TrapKernelHandler;
    InterruptVectorTable[TRAP_CLOCK] = TrapClockHandler;
    InterruptVectorTable[TRAP_ILLEGAL] = TrapIllegalHandler;
    InterruptVectorTable[TRAP_MEMORY] = TrapMemoryHandler;
    InterruptVectorTable[TRAP_MATH] = TrapMathHandler;
    InterruptVectorTable[TRAP_TTY_RECEIVE] = TrapTTYReceiveHandler;
    InterruptVectorTable[TRAP_TTY_TRANSMIT] = TrapTTYTransmitHandler;
    InterruptVectorTable[TRAP_DISK] = TrapDiskHandler;
    WriteRegister(REG_VECTOR_BASE, InterruptVectorTable);
    /* =========== SETUP THE INTERRUPT VECTOR TABLE =========== */

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

    // init a new process for boot

    int num_of_frames = pmem_size / PAGESIZE;
    
    // initialize bit vector, an array of integers of size num_of_frame
    int bit_vector[num_of_frames]; // set to all available initially
    for (int fr_number = 0; fr_number < num_of_frames; fr_number++) {
        bit_vector[fr_number] = PAGE_FREE;
    }

// ================================= //
//   INITIALIZE REGION0 PAGE TABLE   //
// ================================= //

    TracePrintf(0,"DEBUG: Starting to initialize region0 page table\n");

    // number of kernel pagetable entires
    int k_page_table_entries = VMEM_0_SIZE / PAGESIZE;

        // num of entries in page table for virtual memory
    //unsigned int pt_num_entries = ReadRegister(REG_PLTR0);

    pcb_t *boot_process = init_process();

    boot_process->pid = 0; // find this value somewhere
    boot_process->parent = NULL;

    boot_process->user_context = uctxt;
    boot_process->kernel_context = NULL; // null for now. INIT here?

    int kernal_page_table_size = VMEM_0_SIZE / PAGESIZE;
    int userland_page_table_size = VMEM_1_SIZE / PAGESIZE;

    pte_t *region0_pagetable[kernal_page_table_size]; //READREGISTER FUNCTION INSTEAD?
    pte_t *region1_pagetable[userland_page_table_size];

    boot_process->user_page_table = region1_pagetable;
    boot_process->kernal_page_table = region0_pagetable;

    //=== leave user alone for now. but can go here ===//

    
    // interate through each page table entry to index's
    for (int i = 0; i < kernal_page_table_size; i++) {

        //void *lower_addr = VMEM_0_BASE;
        //void *lower_addr = 

        if (i = 0) {
            boot_process->user_text_pt_index = 0; // because this is the bottom;
        }

        if 
    }

    // tell hardware where Region0's page table, (virtual memory base address of page_table)
    WriteRegiter(REG_PTRB0, &k_page_table);

    // tell hardware the number of pages in Region0's page table
    WriteRegister(REG_PTLR0,k_page_table_entries);

// helpful globals/constants

    // question: how does "build process provide" us with these 3 variables
    // void *_kernel_data_start; // lowest addr in kernel data
    // void *_kernel_data_end; // lowest unused address of kernel data,
    // void *_kernel_orig_brk; // address of kernel brk

    // VMEM_0_BASE // bottom address of vmem
    // PMEM_BASE // bottom address of pmem
    // VMEM_1_BASE == VMEM_0_LIMIT
    // VMEM_1_LIMIT // first byte above the region
    // VMEM_1_SIZE
    // VMEM_0_SIZE // size of virtual mem for region 0

    // from 3.5.3
    // KERNEL_STACK_MAXSIZE // fixed max size
    // KERNEL_STACK_BASE // bottom (lower address) of stack
    // KERNEL_STACK_LIMIT // first byte beyond stack (above region 0's vm)

    
    // lowest address in a given page (we'll use in our loop)
    void *page_lowest_addr;
    // first invalid addr above a given page (we'll use in our loop)
    void *page_highest_addr;

    // for each index of pagetable, since page table is indexed by vpn
    // index also acts as vpn and (for kernel only) is equal to pfn
    int index; 
    
    // for each each page table entry
    for (index = 0;index < k_page_table_entries; index++) {
        
        // initialize current page
        page_lowest_addr = VMEM_0_BASE + index * PAGESIZE;
        page_highest_addr = page_lowest_addr + PAGESIZE;
        
    // .text
        // if the page_highest_addr less than or eql to kernel_data_start
        if (page_highest_addr <= kernel_data_start) {

            // create pte with .text permissions
            struct pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_NO_W_X; // we can read and execute our code 
            entry.pfn = index;
            k_page_table_entries[index] = entry;

            // update bitvector
            bit_vector[index] = PAGE_NOT_FREE;
        }
        
    // .data
        // else if the page_lowest_addr above or equal to kernel_data_start 
        // and page_highest_addr less than kernel_data_end
        else if ((page_lowest_addr >= kernel_data_start) && (page_highest_addr < kernel_data_end)) {

            // create pte with .data permissions
            struct pte_t entry;
            entry.valid = 1;
            entry.prot = R_W_NO_X; // we can read, write but not execute our globals
            entry.pfn = index;
            k_page_table_entries[index] = entry;

            // update bitvector
            bit_vector[index] = PAGE_NOT_FREE;
        }

    // heap
        // else if the page_lowest_addr above or equal to kernel_data_end
        // and page_highest_addr less than kernel_orig_brk
        else if ((page_lowest_addr >= kernel_data_end) && (page_highest_addr < _kernel_orig_brk)) {
            
            // create pte with .heap permissions
            struct pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our heap
            entry.pfn = index;
            k_page_table_entries[index] = entry;

            // update bit vector
            bit_vector[index] = PAGE_NOT_FREE;
        }
    
    // stack
        // else if the page_lowest_addr above or equal to kernel_stack_base
        // and page_highest_addr less than stack limit
        else if ((page_lowest_addr >= KERNEL_STACK_BASE) && (page_highest_addr < KERNEL_STACK_LIMIT)) {

            // create pte with stack permissions
            struct pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our stack
            entry.pfn = index;
            k_page_table_entries[index] = entry;

            // update bit vector
            bit_vector[index] = PAGE_NOT_FREE;
        }

        // otherwise
        else {

            //  create an invalid page entry
            struct pte_t entry;
            entry.valid = INVALID_FRAME;
            k_page_table_entries[index] = entry;

            // update bit vector
            bit_vector[index] = PAGE_FREE;
        }
    }
    
    TracePrintf(0,"DEBUG: Done initializing region0 page table\n");

// ================================= //
//   INITIALIZE REGION1 PAGE TABLE   //
// ================================= //

    // number of user pagetable entires
    int u_page_table_entries = VMEM_1_SIZE / PAGESIZE;

    // MAX_PT_LEN is a constant in hardware.h, the max #of pagetable entries
    if (u_page_table_entries < MAX_PT_LEN) {
        TracePrintf(0,"Something went wrong, we have too many page table entries (we have %d, max is %d)", u_page_table_entries,MAX_PT_LEN);
    }

    // define page table, an array of pte's
    struct pte_t u_page_table[u_page_table_entries];
    if (u_page_table_entries = malloc(sizeof(pte_t * u_page_table_entries)) == NULL) {
        TracePrintf(0,"Malloc for user page table failed!\n");
    }

    // tell hardware where Region1's page table, (virtual memory base address of page_table)
    WriteRegiter(REG_PTRB1,&u_page_table);

    // tell hardware the number of pages in Region1's page table
    WriteRegister(REG_PTLR1,u_page_table_entries);

    // when we set up region1 we must do stack only, one valid page for it


    // find the first free frame in region1 memory space, so we start
    // indexing after all of kernel's frame indices
    for (index=k_page_table_entries; index<pmem_size >> PAGESHIFT; index++) {
        if (bit_vector[index] == PAGE_FREE) {

            // create pte with stack permissions
            // I give write permissions now so that we can write our initial code to it
            // this must however be changed afterward!

            // create pte with stack permissions
            struct pte_t entry;
            entry.valid = VALID_FRAME;
            entry.prot = R_W_NO_X; // we can read, write but not execute our stack

            // need to shift our index down by the # of kernel page entries
            entry.pfn = index - k_page_table_entries
            u_page_table[index-k_page_table] = entry

            // update bit vector
            bit_vector[index] = PAGE_NOT_FREE
        }
    }
    TracePrintf(0,"DEBUG: Done initializing region1 page table\n");   
}
