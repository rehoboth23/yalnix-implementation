/*
 *  traphandlers.H
 *  
 *  Defines function handlers for all traps/syscalls as well as other consts required for syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#include <ykernel.h>
#include "pipe.h"
#include "queue.h"


/**
 * @brief Interrupt Vector Table to map traps to functions
 * 
 */
void (*InterruptVectorTable[TRAP_VECTOR_SIZE]) (void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_KERNEL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapKernelHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_CLOCK
 * 
 * @param ctx context from which the trap occured
 */
void TrapClockHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_ILLEGAL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapIllegalHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_MEMORY
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMemoryHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_MATH
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMathHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_RECEIVE
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYReceiveHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_TRANSMIT
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYTransmitHandler(void *ctx);

/**
 * @brief Handler in interrupt vector table for TRAP_DISK
 * 
 * @param ctx user context from which the trap occured
 */
void TrapDiskHandler(void *ctx);