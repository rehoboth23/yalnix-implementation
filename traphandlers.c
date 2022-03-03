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
    UserContext *uctxt = (UserContext *) ctx;
    u_long *regs = uctxt->regs;
    TracePrintf(0,"We got code: %x\n",uctxt->code);
    switch (uctxt->code) {
         case YALNIX_FORK: 
            TracePrintf(0, "kernel calling Fork()\n");
            regs[0] = KernelFork(uctxt);
            break;
        case YALNIX_EXEC:
            TracePrintf(0, "kernel calling Exec(%s, ...)\n", regs[0]);
            regs[0] = KernelExec(uctxt, (char *) regs[0], (char **) regs[1]);
            
            // if kernel exec didn't fail
            if (regs[0] != ERROR) {
                uctxt->sp = activePCB->user_context.sp;
                uctxt->pc = activePCB->user_context.pc;
            }
            // otherwise exit the process with error code
            else {
                KernelExit(ERROR,uctxt);
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
            TracePrintf(0, "kernel calling TtyRead(uctxt, %d, %p, %d)\n", (int) regs[0], (void *) regs[1], (int) regs[2]);
            regs[0] = KernelTtyRead(ctx, (int) regs[0], (void *) regs[1], (int) regs[2]);
            break;
        case YALNIX_TTY_WRITE:
            TracePrintf(0, "kernel calling TtyWrite(uctxt, %d, %p, %d)\n", (int) regs[0], (void *) regs[1], (int) regs[2]);
            regs[0] = KernelTtyWrite(ctx, (int) regs[0], (void *) regs[1], (int) regs[2]);
            break;
        default:
            TracePrintf(0, "Unknown code\n");
            break;
    }
    // check code of uctxt

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
    TracePrintf(0, "Ready -> %d ::: Blocked -> %d ::: Defunct -> %d ::: TtyRead %d ::: TtyWrite %d\n", ready_q->size, blocked_q->size, defunct_q->size, ttyReadQueues[0]->size, ttyWriteQueues[0]->size);
    global_clock_ticks++;
    // if (ready_q->size > 0) { 
    SwapProcess(ready_q,(UserContext *)ctx);
    // }
    int limit = blocked_q->size;
    for (int i = 0; i < limit; i++) {
       pcb_t *pcb = queue_pop(blocked_q);
       if (pcb != NULL) queue_add(CheckBlocked(pcb), pcb, pcb->pid);
    }
    for (int i = 0; i < 4; i++) {
        queue_t *ttyQueue = ttyWriteQueues[i];
        limit = ttyQueue->size;
        if (ttyQueue->size > 0) {
            pcb_t *pcb = queue_peek(ttyQueue);
            queue_add(CheckBlocked(pcb), pcb, pcb->pid);
        }
    }
}

/**
 * @brief Handler in interrupt vector table for TRAP_ILLEGAL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapIllegalHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;
    TracePrintf(0,"TrapIllegalHandler: Illegal Instruction\n");
    KernelExit(0, uctxt);
}

/**
 * @brief Handler in interrupt vector table for TRAP_MEMORY
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMemoryHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;
    int code = uctxt->code;
    void *addr = uctxt->addr;

    if ( (int) addr > VMEM_1_LIMIT || (int) addr < VMEM_0_BASE) {
        TracePrintf(0, "TrapMemoryHandler: Invalid Address\n");
        KernelExit(-1, uctxt);
    }

    switch (code) {
    case YALNIX_MAPERR:
        TracePrintf(0, "TrapMemoryHandler: YALNIX_MAPERR\n");
        KernelExit(-1, uctxt);
    case YALNIX_ACCERR:
        TracePrintf(0, "TrapMemoryHandler: YALNIX_ACCERR\n");
         KernelExit(-1, uctxt);
    default:
        TracePrintf(0, "TrapMemoryHandler: Unknown Memory Error\n");
         KernelExit(-1, uctxt);
    }
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
    UserContext *uctxt = (UserContext *) ctx;
    TracePrintf(0,"TrapMathHandler: Illegal Math Operation\n");
    KernelExit(0, uctxt);
}

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_RECEIVE
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYReceiveHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;

    int tty_id = uctxt->code;
    char *ttyBuffer = ttyReadbuffers[tty_id];
    queue_t *ttyQueue = ttyReadQueues[tty_id];
    TracePrintf(0, "TrapTTYReceiveHandler LOG: track %d\n", ttyReadTrackers[tty_id]);

    // clear stale bytes from the buffer
    // memset(ttyBuffer, 0, TERMINAL_MAX_LINE);
    char tempBuffer[TERMINAL_MAX_LINE];

    // use hardware function to read into the buffer
    int bytes = TtyReceive(tty_id, tempBuffer, TERMINAL_MAX_LINE);
    int to_copy;
    if (TERMINAL_MAX_LINE - ttyReadTrackers[tty_id] < bytes) {
        to_copy = TERMINAL_MAX_LINE - ttyReadTrackers[tty_id];
    } else {
        to_copy = bytes;
    }
    memcpy(ttyBuffer + ttyReadTrackers[tty_id], tempBuffer, to_copy);
    ttyReadTrackers[tty_id] = ttyReadTrackers[tty_id] + to_copy;
    TracePrintf(0, "TrapTTYReceiveHandler LOG: track %d\n", ttyReadTrackers[tty_id]);
    if (ttyQueue->size > 0) {
        pcb_t *nextReader = queue_pop(ttyQueue);
        nextReader->blocked_code = NOT_BLOCKED;
        queue_add(ready_q, nextReader, nextReader->pid);
    }
}

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_TRANSMIT
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYTransmitHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;
    int tty_id = uctxt->code;
    ttyWriteTrackers[tty_id] = TERMINAL_OPEN;
}

/**
 * @brief Handler in interrupt vector table for TRAP_DISK
 * 
 * @param ctx user context from which the trap occured
 */
void TrapDiskHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;
    TracePrintf(0,"Sorry, this trap handler hasn't been implemented yet.\n");
    // no need to worry about, this is extra functionality
    // to do with disk
}