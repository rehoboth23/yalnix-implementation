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
        case YALNIX_PIPE_INIT:
            TracePrintf(0, "kernel calling PipeInit(%p)\n",regs[0]);
            regs[0] = KernelPipeInit((int *)regs[0]);
            break;
        case YALNIX_PIPE_READ:
            TracePrintf(0, "kernel calling PipeRead(%d,%p,%d)\n",(int) regs[0],regs[1], (int) regs[2]);
            regs[0] = KernelPipeRead((int) regs[0], (void *)regs[1],(int) regs[2],ctx);
            break;
        case YALNIX_PIPE_WRITE:
            TracePrintf(0, "kernel calling PipeWrite(%d,%p,%d)\n",regs[0],regs[1],regs[2]);
            regs[0] = KernelPipeWrite((int) regs[0], (void *)regs[1],(int) regs[2],ctx);
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
   
}

/**
 * @brief Handler in interrupt vector table for TRAP_ILLEGAL
 * 
 * @param ctx user context from which the trap occured
 */
void TrapIllegalHandler(void *ctx) {
    TracePrintf(0, "Trap Illegal Handler Called.\n");
    UserContext *user_context = (UserContext *) ctx;

    if (activePCB == NULL) {
        TracePrintf(0,"Error in Trap Illegal Handler, activePCB is null.\n");
        return;
    }

    // set error code
    activePCB->exit_code = ERROR;

    // move process to defunct queue
    SwapProcess(defunct_q, user_context);
}

/**
 * @brief Handler in interrupt vector table for TRAP_MEMORY
 * 
 * @param ctx user context from which the trap occured
 */
/**
 * @brief Handler in interrupt vector table for TRAP_MEMORY
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMemoryHandler(void *ctx) {
    TracePrintf(0, "Trap Memory Hander Called.\n");
    UserContext *user_context = (UserContext *) ctx;

    if (user_context == NULL) {
        TracePrintf(0,"Error in Trap Memory Handler, user_context is null.\n");
        return;
    }

    if (activePCB == NULL) {
        TracePrintf(0,"Error in Trap Memory Handler, activePCB is null.\n");
        return;
    }

    // get indexes for the user page base and stack index
    int user_page_base = VMEM_1_BASE >> PAGESHIFT;
    int stack_index = activePCB->user_stack_pt_index;

    // get the page of the target address
    int target = ( DOWN_TO_PAGE( (int)user_context->addr) >> PAGESHIFT ) - user_page_base;

    TracePrintf(0, "Trap Memory user page base is %d, stack index is %d, address is %d, and target is %d\n", user_page_base, stack_index, (int) user_context->addr, target);

    // Check to make sure the target address is not withing a valid frame (aka region1 heap or below)
    if (activePCB->user_page_table[target].valid == VALID_FRAME || target < 0) {
        TracePrintf(0, "Current process wishes to move stack to occupied frame. Aborting.");

        activePCB->exit_code = ERROR;
        SwapProcess(defunct_q, user_context);
        return;
    }

    // from the target frame up to stack index, allocate frames for stack
    for (int i = target; i < stack_index; i++) {
        if (activePCB->user_page_table[i].valid == INVALID_FRAME) {
            TracePrintf(0, "index -> %d\n", i);
            int pfn = AllocatePFN();
            if (pfn == ERROR) {
                return;
            }

            // update page table entry for stack
            activePCB->user_page_table[i].prot = NO_X_W_R;
            activePCB->user_page_table[i].pfn = pfn;
            activePCB->user_page_table[i].valid = VALID_FRAME;
        }
    }
}

/**
 * @brief Handler in interrupt vector table for TRAP_MATH
 * 
 * @param ctx user context from which the trap occured
 */
void TrapMathHandler(void *ctx) {
    TracePrintf(0, "Trap Math Handler Called.\n");
    UserContext *user_context = (UserContext *) ctx;

    // error check
    if (activePCB == NULL) {
        TracePrintf(0,"Error in Trap Math Handler, activePCB is null.\n");
        return;
    }

    // give an error exit code
    activePCB->exit_code = ERROR;

    // move process to defunct queue
    SwapProcess(defunct_q, user_context);
}

/**
 * @brief Handler in interrupt vector table for TRAP_TTY_RECEIVE
 * 
 * @param ctx user context from which the trap occured
 */
void TrapTTYReceiveHandler(void *ctx) {
    UserContext *uctxt = (UserContext *) ctx;

    int tty_id = uctxt->code;
    // buffers to store for each terminal
    char *ttyBuffer = ttyReadbuffers[tty_id];
    queue_t *ttyQueue = ttyReadQueues[tty_id];

    // clear stale bytes from the buffer
    // memset(ttyBuffer, 0, TERMINAL_MAX_LINE);
    char tempBuffer[TERMINAL_MAX_LINE];

    // use hardware function to read into a temporary buffer
    int bytes = TtyReceive(tty_id, tempBuffer, TERMINAL_MAX_LINE);
    int to_copy;

    // if we have space in our buffer
    if (TERMINAL_MAX_LINE - ttyReadTrackers[tty_id] < bytes) {

        // set bytes to copy to be the current space we have left
        to_copy = TERMINAL_MAX_LINE - ttyReadTrackers[tty_id];
    } else {
        // write all that we can
        to_copy = bytes;
    }

    memcpy(ttyBuffer + ttyReadTrackers[tty_id], tempBuffer, to_copy);

    // update readtrackers
    ttyReadTrackers[tty_id] = ttyReadTrackers[tty_id] + to_copy;

    // if anyone else if waiting to read, wake them up
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

    // mark the terminal as available
    ttyWriteTrackers[tty_id] = TERMINAL_OPEN;

    // check the queue of processes waiting to write
    queue_t *ttyQueue = ttyWriteQueues[tty_id];
    pcb_t *pcb = queue_peek(ttyQueue);

    // wake the first one up if it isn't the active process
    // ( put it into ready queue )
    if (ttyQueue->size > 0 && activePCB->pid != pcb->pid) {
        queue_add(CheckBlocked(pcb), pcb, pcb->pid);
    }
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