/*
 *  traphandlers.c
 *  
 *  Defines function handlers for all traps/syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#include <yalnix.h>
#include <ykernel.h>
#include "queue.h"
#include "kernel.h"
#include "traphandlers.h"


void (*InterruptVectorTable[TRAP_VECTOR_SIZE]) (void *ctx);


// ********************************************************** 
//                      Trap Handlers
// **********************************************************

/**
 * @brief Handler in interrupt vector table for TRAP_KERNEL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapKernelHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    u_long *regs = user_context->regs;
    TracePrintf(0,"We got code: %x\n",user_context->code);
    switch (user_context->code) {
         case YALNIX_FORK: 
            TracePrintf(0, "kernel calling Fork()\n");
            regs[0] = KernelFork(user_context);
            break;
        case YALNIX_EXEC:
            TracePrintf(0, "kernel calling Exec(%s, ...)\n", regs[0]);
            regs[0] = KernelExec(user_context, (char *) regs[0], (char **) regs[1]);
            
            // if kernel exec didn't fail
            if (regs[0] != ERROR) {
                user_context->sp = activePCB->user_context.sp;
                user_context->pc = activePCB->user_context.pc;
            }
            // otherwise exit the process with error code
            else {
                KernelExit(ERROR,user_context);
            }
            
            break;
        case YALNIX_EXIT:
            TracePrintf(0, "kernel calling Exit(%d)\n", (int) regs[0]);
            KernelExit((int) regs[0],ctx);
            break;
        case YALNIX_WAIT:
            TracePrintf(0, "kernel calling Wait()\n");
            regs[0] = KernelWait((int *) regs[0],ctx);
            break;
        case YALNIX_GETPID:
            TracePrintf(0, "kernel calling GetPid()\n");
            regs[0] = KernelGetPid();
            break;
        // case YALNIX_GETPPID:
        //     TracePrintf(0, "kernel calling GetPPid()\n");
        //     regs[0] = KernelGetPPid();
        //     break;
        case YALNIX_BRK:
            TracePrintf(0, "kernel calling Brk(%d)\n", regs[0]);
            regs[0] = KernelBrk((void *) regs[0]);
            break;
        case YALNIX_DELAY:
            TracePrintf(0, "kernel calling Delay(%d)\n", regs[0]);
            regs[0] = KernelDelay(regs[0],ctx);
            break;
        case YALNIX_TTY_READ:
            TracePrintf(0, "kernel calling TtyRead()\n");
            break;
        case YALNIX_TTY_WRITE:
            TracePrintf(0, "kernel calling TtyWrite()\n");
            break;
        default:
            TracePrintf(0, "Unknown code\n");
            break;
    }
    // check code of user_context

    // depending on code, call the corresponding function below

    // arguments are in regs[] in usercontext, starting with regs[0]

    // return arguments should be in regs[0]
}

/**
 * @brief Handler in interrupt vector table for TRAP_CLOCK
 * 
 * @param ctx context from which the trap occured
 */
void TrapClockHandler(void *ctx) {
    TracePrintf(0, "Clock Tick -> %d\n", global_clock_ticks);
    TracePrintf(0, "Ready -> %d ::: Blocked -> %d ::: Defunct -> %d\n", ready_q->size, blocked_q->size, defunct_q->size);
    global_clock_ticks++;
    SwapProcess(ready_q,(UserContext *)ctx);
    int limit = blocked_q->size;
    for (int i = 0; i < limit; i++) {
       pcb_t *pcb = queue_pop(blocked_q);
       if (pcb != NULL) queue_add(CheckBlocked(pcb), pcb, pcb->pid);
    }
}

/**
 * @brief Handler in interrupt vector table for TRAP_ILLEGAL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapIllegalHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // same as TrapMathHandler
}

/**
 * @brief Handler in interrupt vector table for TRAP_MEMORY
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMemoryHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // make sure user context is valid
    
    // chcek if this is an implicit request
    // to enlarge memory on process's stack
            // question: how to check if implicit req?
    // if so: enlarge stack to cover the address in the addr
    // field os UserContext, then return

    // otherwise, same as TrapMathHandler
}

/**
 * @brief Handler in interrupt vector table for TRAP_MATH
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMathHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // get current running process from running queue
    // check that it's valid

    // abort it
    // get another ready process to run (or do we call some function to do this?)
}

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_RECEIVE
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYReceiveHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // check that usercontext is valid

    // code of usercontext is the terminal has a new line
    // call TtyReceive from hardware.h to read input from terminal
    
    // buffer input line for more user TtyRead syscalls if necessary
    
}

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_TRANSMIT
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYTransmitHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // check that usercontext is valid

    // code field of usercontext is the terminal that completed
    //a TtyTransmit operation

    // if process that started terminal output is blocked, finish it
    // start the next terminal output on this terminal if there are any

}

/**
 * @brief Handler in interrupt vector table for TRAP_DISK
 * 
 * @param ctx user context from which the trap occured
 */
void TrapDiskHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // no need to worry about, this is extra functionality
    // to do with disk
}