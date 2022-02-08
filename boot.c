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
#include "interrupt.h"

extern handler_func_t InterruptVectorTable[TRAP_VECTOR_SIZE];


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

    // loop through cmd_args until null

        // == common yalnix options: == //
        // -x

        //-lk

        //-lh

        //-lu

        //-W

        // == esoteric yalnix options == //
        // -t tracefile

        // -C NNN

        // -In file
        
        // -On file

    // if there were no arguments, look for an executable called init and run that

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
