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

#include "process.h"

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

    // init a new process for boot
    


    int num_of_frames = pmem_size / PAGESIZE
    
    // initialize bit vector, an array of integers of size num_of_frame
    int bit_vector[num_of_frames] = {0};

    //==initialize region 0's page table==

        // virtual memory base address of page table
    unsigned int pt_vmem_base = ReadRegister(REG_PTRB0);

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
