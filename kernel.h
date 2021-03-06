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
#include "pipe.h"
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
int SetRegion0_pt(pte_t *k_pt, int k_pt_size);

/**
 * @brief function to create the VM for region1 pagetable
 * 
 * @param u_pt user page table
 * @param u_pt_size user page table size
 * @param uctxt user context
 */
int SetRegion1_pt(pte_t *u_pt, int u_pt_size);

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
int SetUpGlobals(); 

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
int DeallocatePFN(int pfn);

/**
 * @brief 
 * 
 * @param uctxt 
 */
int SwapProcess(queue_t *moveActive,UserContext *uctxt);

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

#define MAX_LOCKS 100
#define MAX_CVARS 5000

// tracefile that traceprint writes to
extern char* tracefile; //= TRACE;
// process that's currently active
extern pcb_t *activePCB;
// global queues
extern queue_t *ready_q;
extern queue_t *blocked_q;
extern queue_t *defunct_q;
// keeping track of free page frame numbers
extern list_t *pfn_list;
// clock ticks
extern int global_clock_ticks;
extern pcb_t *idlePCB;
// terminal helpers
extern queue_t *ttyReadQueues[NUM_TERMINALS];
extern queue_t *ttyWriteQueues[NUM_TERMINALS];
extern char *ttyReadbuffers[NUM_TERMINALS];
extern int ttyWriteTrackers[NUM_TERMINALS];
extern int ttyReadTrackers[NUM_TERMINALS];
// parent pipe, the pipe with id 0
extern pipe_t *head_pipe;
// locks and cvars
extern list_t *lock_list;
extern int lock_status[MAX_LOCKS];
extern list_t *cvar_list;
extern int cvar_status[MAX_CVARS];
extern queue_t *lockAquireQueues[MAX_LOCKS];
extern queue_t *cvarWaitQueues[MAX_CVARS];


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
