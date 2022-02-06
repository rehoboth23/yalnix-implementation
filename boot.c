/*
 *  boot.c
 *  
 *  contains the KernelStart function, boots and intializes
 *  our OS and its first process
 */

#include "include/ykernel.h"
#include "include/hardware.h"
#include "include/ylib.h"
#include "include/yuser.h"

enum {
    DEFAULT_TRACE_LEVEL   =   1,
    MAX_TRACE_LEVEL       =  10, // yo what is the max trace level am I blind
    DEFAULT_TICK_INTERVAL =  400, // in ms !
    PAGE_FREE             =   1,
    PAGE_NOT_FREE         =   0
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
char* tracefile = TRACE;

// tick interval of clock
int tick_interval = DEFAULT_TICK_INTERVAL;



/*
 * KernelStart
 *
 * initializes our OS and its first process (ADD BETTER DESCRIPTION HERE)
 *
 * takes in
 *  - cmd_args[], arguments from cmdline
 *  - pmem_size, size of physical memory for our OS, it's in BYTES
 *  - uctxt, usercontext to go into our idle process
 * 
 */
void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt) {

// ====================================== //
//   HANDLING SWITCHES IN CMDLINE INPUT 
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

        //-lk  --> TODO: handle case where argument after this switch is another switch/ some string
        else if (strcmp(cmd_args[index],"-lk") == 0) {
            TracePrintf(0,"-lk switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the kernel
                    k_tracing_level = level;

                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n",level);
                
            } else TracePrintf(0,"Error, expected level after -lk switch\n");
        }

        //-lh  --> TODO: handle case where argument after this switch is another switch/ some string
        else if (strcmp(cmd_args[index],"-lh") == 0) {
            TracePrintf(0,"-lh switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the hardware
                    h_tracing_level = level;


                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n",level);
                
            } else TracePrintf(0,"Error, expected level after -lh switch\n");
        }

        //-lu  --> TODO: handle case where argument after this switch is another switch/ some string
        else if (strcmp(cmd_args[index],"-lu") == 0) {
            TracePrintf(0,"-lu switch detected\n");

            // get level by incrementing index
            index++;

            // check if the next input is NULL
            if (cmd_args[index] != NULL) {

                int level = atoi(cmd_args[index]);

                // make sure level is in range
                if (level >= 0 && level <= MAX_TRACE_LEVEL) {

                    // if valid, set tracing level of the user
                    u_tracing_level = level;


                } else TracePrintf(0,"Error, invalid tracing level, we don't accept %d\n",level);
                
            } else TracePrintf(0,"Error, expected level after -lu switch\n");
        }

        //-W
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


                } else TracePrintf(0,"Error, invalid tick interval, we don't accept %d\n",new_tick_interval);
                
            } else TracePrintf(0,"Error, expected tick interval after -C switch\n");


        }
        
        // -In file <-- TODO
            // check if first char of arg is -I
            // check that integer next to it is valid
            // check existence of file

            // if all good, feed terminal n with data from file
            // as if it were typed there
        
        // -On file <-- TODO
            // check if first char of arg is -O
            // check that integer next to it is valid
            // check existence of file

            // if all good, feed output of terminal n to 
            // filename TTYLOG.n, or change that to file

        // TODO: or if there is a valid string, then it's the initial process
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
        TracePrintf(0,"Error! inputted pmem_size (%d) is negative or 0!\n",pmem_size);
    }

    // get num_of_frame (pmem_size / PAGESIZE)
        // pmem_size is an arg, PAGESIZE is in hardware.h
        // they're both in bytes
    // number of frames == number of pages
    int num_of_frames = pmem_size / PAGESIZE
    
    // initialize bit vector, an array of integers of size num_of_frame
    int bit_vector[num_of_frames] = {0};

// ======================
//   INITIALIZE REGION0
// ======================

    // number of kernel pagetable entires
    int k_page_table_entries = VMEM_0_SIZE / PAGESIZE;

    // MAX_PT_LEN is a constant in hardware.h, the max #of pagetable entires
    if (k_page_table_entries < MAX_PT_LEN) {
        TracePrintf(0,"Something went wrong, we have too many page table entries (we have %d, max is %d)",k_page_table_entries,MAX_PT_LEN);
    }

    // define page table
    struct pte_t k_page_table[k_page_table_entries];

    // tell hardware where Region0's page table, (virtual memory base address of page_table)
    WriteRegiter(REG_PTRB0,&k_page_table);

    // tell hardware the number of pages in Region0's page table
    WriteRegister(REG_PTLR0,k_page_table_entries);

    // TODO: find out how "build process provides" us with this
    void *_kernel_data_start; // lowest addr in kernel data
    void *_kernel_data_end; // lowest unused address of kernel data,
    void *_kernel_orig_brk; // address of kernel brk

    VMEM_0_BASE // bottom address of vmem
    PMEM_BASE // bottom address of pmem

    // TODO find permissions for kernel data, heap, and text, and adjust page tables differently

    // from 3.5.3
    KERNEL_STACK_MAXSIZE // fixed max size
    KERNEL_STACK_BASE // bottom (lower address) of stack
    KERNEL_STACK_LIMIT // first byte beyond stack (above region 0's vm)

    
    // lowest address in a given page (we'll use in our loop)
    void *page_lowest_addr;
    // first invalid addr above a given page (we'll use in our loop)
    void *page_highest_addr;

    int index; // indexing through each page table entry
    for (index = 0;index < k_page_table_entries; index++) {
        
        // initialize current page
        page_lowest_addr = VMEM_0_BASE + index * PAGESIZE
        page_highest_addr = page_lowest_addr + PAGESIZE;
        
        // if the page_highest_addr leess than kernel_data_start
        if (page_highest_addr < kernel_data_start) {

            // TODO create pte with .text permissions
                    
            // update bitvector
            bit_vector[index] = PAGE_NOT_FREE;
        }
            
        // else if the page_lowest_addr above or equal to kernel_data_start 
        // and page_highest_addr less than kernel_data_end
        else if ((page_lowest_addr >= kernel_data_start) && (page_highest_addr < kernel_data_end)) {

            // TODO create pte with .data permissions

            // update bitvector
            bit_vector[index] = PAGE_NOT_FREE;
        }

        // else if the page_lowest_addr above or equal to kernel_data_end
        // and page_highest_addr less than kernel_orig_brk
        else if ((page_lowest_addr >= kernel_data_end) && (page_highest_addr < _kernel_orig_brk)) {
            
            // TODO create pte with .heap permissions

            // update bit vector
            bit_vector[index] = PAGE_NOT_FREE;
        }
            
        // else if the page_lowest_addr above or equal to kernel_stack_base
        else if ((page_lowest_addr >= KERNEL_STACK_BASE) && (page_highest_addr < KERNEL_STACK_LIMIT))

            // TODO create pte with stack permissions

            // update bit vector
            bit_vector[index] = PAGE_NOT_FREE

        // otherwise
        else {

            // TODO create an invalid page entry

            // update bit vector
            bit_vector[index] = PAGE_FREE;
        }
            

    }
   


    //==initialize region 1's page table==

    VMEM_1_BASE == VMEM_0_LIMIT
    VMEM_1_LIMIT // first byte above the region
    VMEM_1_SIZE
    VMEM_0_SIZE // size of virtual mem for region 0

        
}
