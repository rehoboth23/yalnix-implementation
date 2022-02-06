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
    DEFAULT_TICK_INTERVAL =  400 // in ms !
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

    // get num_of_frame (pmem_size / PAGESIZE)
        // pmem_size is an arg, PAGESIZE is in hardware.h
        // they're both in bytes
    if (pmem_size <= 0) {
        // ERROR
    }

    int num_of_frames = pmem_size / PAGESIZE
    
    // initialize bit vector, an array of integers of size num_of_frame
    int bit_vector[num_of_frames] = {0};

    //==initialize region 0's page table==

        // virtual memory base address of page table
    unsigned int pt_vmem_base = ReadRegister(REG_PTRB0);

        // num of entries in page table for virtual memory
    unsigned int pt_num_entries = ReadRegister(REG_PLTR0);

    void *_kernel_data_start; // lowest addr in kernel data
    void *_kernel_data_end; // lpwest unused address of kernel data
    void *_kernel_orig_brk; // address of kernel brk
    VMEM_0_BASE // bottom address of vmem
    PMEM_BASE // bottom address of pmem

    // find permissions for kernel data, heap, and text, and adjust page tables differently

    // HOW DO WE DO KERNEL STACK
    KERNEL_STACK_MAXSIZE
    KERNEL_STACK_BASE
    KERNEL_STACK_LIMIT // 

    // for each frame that fits between kernel_base and kernel_data_start
        // allocate a pte from 0 to i
    
    // for each frame that fits between kernel_data_base to kernel_dat_end

    //==initialize region 1's page table==

    VMEM_1_BASE == VMEM_0_LIMIT
    VMEM_1_LIMIT // first byte above the region
    VMEM_1_SIZE
    VMEM_0_SIZE // size of virtual mem for region 0

        
}
