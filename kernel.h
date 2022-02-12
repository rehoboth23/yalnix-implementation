/*
 *  boot.h
 *  
 *  contains the globals for the yalnix OS kernel
 *
 *  our OS and its first process
 */


#include <hardware.h>

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
 * @param bit_vector bit vector array
 */
void SetRegion0_pt(pte_t *k_pt, int k_pt_size, int bit_vector[]);

/**
 * @brief function to create the VM for region1 pagetable
 * 
 * @param u_pt user page table
 * @param u_pt_size user page table size
 * @param bit_vector bit vector array
 * @param uctxt user context
 */
void SetRegion1_pt(pte_t *u_pt, int u_pt_size, int bit_vector[],UserContext *uctxt);

/**
 * @brief function for idle process representing kernel process
 * 
 */
void DoIdle(void);

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
#define k_tracing_level DEFAULT_TRACE_LEVEL;  // kernel tracing level
#define h_tracing_level DEFAULT_TRACE_LEVEL;  // hardware tracing level
#define u_tracing_level DEFAULT_TRACE_LEVEL;  // user tracing level

// tracefile that traceprint writes to
char* tracefile; //= TRACE;

// tick interval of clock
#define tick_interval DEFAULT_TICK_INTERVAL;