/*
 *  boot.h
 *  
 *  contains the globals for the yalnix OS kernel
 *
 *  our OS and its first process
 */

#ifndef __KERNEL_H_
#define __KERNEL_H_

#include <hardware.h>
#include "queue.h"
#include "process.h"
#include "include.h"
#include "list.h"

/**
 * @brief initializes our OS: page tables for region0 and region1
 * accepts configurations through cmdline switches, and starts our first
 * process
 * 
 * @param cmd_args arguments from cmdline
 * @param pmem_size pmem_size, size of physical memory for our OS, it's in BYTES
 * @param uctxt uctxt, usercontext to go into our idle process
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt);

/**
 * @brief function to create the VM for region0 pagetable
 * 
 * @param k_pt kernel page table
 * @param k_pt_size kernel page table size
 */
void SetRegion0_pt(pte_t *k_pt, int k_pt_size);

/**
 * @brief function to create the VM for region1 pagetable
 * 
 * @param u_pt user page table
 * @param u_pt_size user page table size
 * @param uctxt user context
 */
void SetRegion1_pt(pte_t *u_pt, int u_pt_size);

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
int SetKernelBrk(void* addr);

/**
 * @brief 
 * 
 * @param kctxt 
 * @param currPCB 
 * @param newPCB 
 * @return KernelContext* 
 */
KernelContext *KCCopy(KernelContext *kc_in, void *pcb, void *notUsed);

/**
 * @brief 
 * 
 * @param kc_in 
 * @param pcb1 
 * @param pcb2 
 * @return KernelContext* 
 */
KernelContext *KCSwitch(KernelContext *kc_in, void *pcb1, void *pcb2);


/**
 * @brief Set the Up Globals object
 * 
 */
void SetUpGlobals(); 

/*
 * ==>> Declare the argument "proc" to be a pointer to the PCB of
 * ==>> the current process.
 */
int LoadProgram(char *name, char *args[], pcb_t *proc);


/**
 * @brief Get a Free PFN
 * 
 * @return int -1 if not free pfn, pfn otherwise
 */
int AllocatePFN();

/**
 * @brief 
 * 
 * @param pfn 
 */
void DeallocatePFN(int pfn);

/**
 * @brief 
 * 
 * @param uctxt 
 */
void SwapProcess(queue_t *moveActive,UserContext *uctxt);

/**
 * @brief 
 * 
 * @param pcb 
 * @return queue_t* 
 */
queue_t *CheckBlocked(pcb_t *pcb);


    
// initialize pointer to bit vector (an array of integers of size num_of_frame)
extern int num_of_frames;

// kernel brk CHECK: if I change kernel_brk does _kernel_orig_brk change too
extern void *kernel_brk;

// =================================
//  yalnix-switch-configured values
// =================================
// TODO for all of these ^^: find out where we have to put these variables to cause actual change

// tracing levels for kernel, hardware, and user
#define k_tracing_level DEFAULT_TRACE_LEVEL;  // kernel tracing level
#define h_tracing_level DEFAULT_TRACE_LEVEL;  // hardware tracing level
#define u_tracing_level DEFAULT_TRACE_LEVEL;  // user tracing level

// tracefile that traceprint writes to
extern char* tracefile; //= TRACE;
extern pcb_t *activePCB;
extern queue_t *ready_q;
extern queue_t *blocked_q;
extern queue_t *defunct_q;
extern list_t *pfn_list;
extern int global_clock_ticks;
extern pcb_t *idlePCB;

// tick interval of clock
#define tick_interval DEFAULT_TICK_INTERVAL;


/*
 * ==>> #include anything you need for your kernel here
 */


/*
 *  Load a program into an existing address space.  The program comes from
 *  the Linux file named "name", and its arguments come from the array at
 *  "args", which is in standard argv format.  The argument "proc" points
 *  to the process or PCB structure for the process into which the program
 *  is to be loaded.
 */

int create_region0_pagetable(pcb_t *proc);
int create_region1_pagetable(pcb_t *proc);

#endif