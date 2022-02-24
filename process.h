/*
 *  process.h
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "hardware.h"
#include "include.h"

typedef struct PCB {
    u_long pid; // pid
    u_long ppid; // parent pid

    int num_children; // number of children
    int clock_ticks;
    int exit_code;

    // context information for process
    UserContext user_context; // hardware.h provides UserContext
    KernelContext kernel_context;  // hardware.h provides KernelContext

    // process user address space info, this is the FIRST ADDRESS of each segment
    pte_t user_page_table[USER_PT_SIZE]; // region 1

    // user address space information (will be needed for brk etc)
    int user_stack_pt_index;
    int user_heap_pt_index;
    int user_text_pt_index;
    int user_data_pt_index;

    pte_t kernel_stack_pt[KERNEL_STACK_SIZE];    // kernel stack for process

    int blocked_code; // code for why the process is blocked
    int return_code;

} pcb_t;

/**
 * @brief 
 * 
 * @param uctxt 
 * @return pcb_t* 
 */
pcb_t *init_process(UserContext *uctxt);

/**
 * @brief 
 * 
 * @param u_pt 
 * @param k_stack 
 * @return int
 */
int free_addr_space(pte_t *u_pt, pte_t *k_stack);

/**
 * @brief 
 * 
 * @param u_pt1 
 * @param u_pt2 
 * @param k_pt 
 * @param reserved_kernel_index 
 * @return int 
 */
int CopyUPT(pte_t *u_pt1, pte_t *u_pt2, pte_t *k_pt, int reserved_kernel_index);

/**
 * @brief 
 * 
 * @param pcb 
 * @return int
 */
int delete_process(pcb_t *pcb);

#endif