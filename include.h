#ifndef __INCLUDE_
#define __INCLUDE_

#include "hardware.h"

#define KERNEL_PT_SIZE     VMEM_0_SIZE / PAGESIZE
#define USER_PT_SIZE       VMEM_1_SIZE / PAGESIZE
#define KERNEL_STACK_SIZE  ( (int) (KERNEL_STACK_LIMIT - KERNEL_STACK_BASE) >> PAGESHIFT )

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
    ADDR_SPACE_ENTRY_SIZE =    4,

    // permissions for page table, in order X W R -- NOT R W X >:(
    
    
    X_NO_W_R              =    5,      // read allowed, no write, exec allowed
    NO_X_NO_W_NO_R        =    0,      // no read, no write, no execUserContext 
    X_W_R                 =    7,      // read allowed, write allowed, exec allowed
    NO_X_W_R              =    3,      // read allowed, write allowed, no exec
    NO_X_NO_W_R           =    1,
    NO_X_W_NO_R           =    2,
    X_W_NO_R              =    6,

    // BLOCKED CODES
    NOT_BLOCKED           =    0,
    BLOCKED_DELAY         =    1, 
    BLOCKED_WAIT          =    2,
    BLOCKED_TTY_READ      =    3,
    BLOCKED_TTY_WRITE     =    4,
    BLOCKED_TTY_TRANSMIT  =    5,

    // TTY I/O 
    TERMINAL_OPEN         =    1,
    TERMINAL_CLOSED       =    0,
};

#endif