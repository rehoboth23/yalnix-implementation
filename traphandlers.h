/*
 *  traphandlers.H
 *  
 *  Defines function handlers for all traps/syscalls as well as other consts required for syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#ifndef __TRAPHANDLERS_H_
#define __TRAPHANDLERS_H_

#include <ykernel.h>
#include "queue.h"

/**
 * @brief Interrupt Vector Table to map traps to functions
 * 
 */
extern void (*InterruptVectorTable[TRAP_VECTOR_SIZE]) (void *ctx);

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


/**
 * @brief 
 * 
 * @param uctxt 
 * @return int 
 */
int KernelFork(UserContext *uctxt);

/**
 * @brief 
 * 
 * @param uctxt 
 * @param filename 
 * @param argvec 
 * @return int 
 */
int KernelExec(UserContext *uctxt, char *filename, char **argvec);

/**
 * @brief 
 * 
 * @param exit_code 
 */
int KernelExit(int exit_code, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param status_ptr 
 * @return int 
 */
int KernelWait(int *status_ptr, UserContext *uctxt);

/**
 * @brief Get the Pid object
 * 
 * @return int 
 */
int KernelGetPid(void);

/**
 * @brief Get the PPid of process
 * 
 * @return int 
 */
int KernelGetPPid(void);

/**
 * @brief 
 * 
 * @param addr 
 * @return int 
 */
int KernelBrk(void *addr);

/**
 * @brief 
 * 
 * @param clock_ticks 
 * @return int 
 */
int KernelDelay(int clock_ticks, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param uctxt
 * @param tty_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelTtyRead(UserContext *uctxt, int tty_id, void *buf, int len);

/**
 * @brief 
 * 
 * @param uctxt
 * @param tty_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelTtyWrite(UserContext *uctxt, int tty_id, void *buf, int len);

/**
 * @brief 
 * 
 * @param pipe_idp 
 * @return int 
 */
int KernelPipeInit(int *pipe_idp);

/**
 * @brief 
 * 
 * @param pipe_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelPipeRead(int pipe_id, void *buf, int len,UserContext *uctxt);

/**
 * @brief 
 * 
 * @param pipe_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelPipeWrite(int pipe_id, void *buf, int len, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param lock_idp 
 * @return int 
 */
int KernelLockInit(int *lock_idp);

/**
 * @brief 
 * 
 * @param lock_id 
 * @param uctxt 
 * @return int 
 */
int KernelAcquire(int lock_id, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param lock_id 
 * @return int 
 */
int KernelRelease(int lock_id);

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @param uctxt 
 * @return int 
 */
int KernelCvarSignal(int cvar_idp, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @param uctxt 
 * @return int 
 */
int KernelCvarBroadcast(int cvar_idp, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @param lock_id 
 * @param uctxt 
 * @return int 
 */
int KernelCvarWait(int cvar_idp, int lock_id, UserContext *uctxt);

/**
 * @brief 
 * 
 * @param id 
 * @return int 
 */
int KernelReclaim(int id);


#endif