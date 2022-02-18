/*
 *  traphandlers.c
 *  
 *  Defines function handlers for all traps/syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#include <ykernel.h>
#include "pipe.h"
#include "queue.h"
#include "contextswitch.h"

extern queue_t* running_q;
extern queue_t* ready_q; 
extern queue_t* blocked_q;
extern queue_t* defunct_q;

extern int *bit_vector;

enum {
    // default values
    PAGE_FREE             =    1,
    PAGE_NOT_FREE         =    0,
    VALID_FRAME           =    1,
    INVALID_FRAME         =    0
};

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
    TracePrintf(0,"We got code: %d\n",user_context->code);
    switch (user_context->code) {
        case YALNIX_FORK: 
            TracePrintf(0, "kernel calling Fork()\n");
            break;
        case YALNIX_EXEC:
            TracePrintf(0, "kernel calling Exec()\n");
            break;
        case YALNIX_EXIT:
            TracePrintf(0, "kernel calling Exir()\n");
            break;
        case YALNIX_WAIT:
            TracePrintf(0, "kernel calling Wait()\n");
            break;
        case YALNIX_GETPID:
            TracePrintf(0, "kernel calling GetPid()\n");
            break;
        case YALNIX_BRK:
            TracePrintf(0, "kernel calling Brk()\n");
            break;
        case YALNIX_DELAY:
            TracePrintf(0, "kernel calling Delay()\n");
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
    TracePrintf(0,"==Trap Clock Handler called!==\n");

    // check running queue
    int size = queue_size(running_q);
    TracePrintf(0,"\tRunning queue size %d\n",size);
    if (size != 1) {
        TracePrintf(0,"ERROR in GetPid, how do we have a running queue of size %d?\n",size);
        Halt();
    }

    // get current proc, add it back to running queue as well
    pcb_t *curr_proc = queue_pop(running_q);
    queue_add(running_q,curr_proc,curr_proc->pid);

    // check blocked queue, decrement if any are delayed
    size  = queue_size(blocked_q);
    TracePrintf(0,"\tBlocked queue size %d\n",size);
    if (size != 0) {
        // for each blocked process
        for (int index = 0 ; index < size ; index++ ) {

            // decrement the delay clock if it isn't 0
            pcb_t *blocked_proc = queue_pop(blocked_q);
            if (blocked_proc->delay_clock > 0) {

                TracePrintf(0,"\tDecrementing delay clock for process %d\n",blocked_proc->pid);
                blocked_proc->delay_clock--;
                
                // add it back to blocked queue
                queue_add(blocked_q,blocked_proc,blocked_proc->pid);
            }
            // move it to ready queue if its delay clock is 0
            else if (blocked_proc->delay_clock == 0) {
                TracePrintf(0,"\tMoving process %d to ready queue\n",blocked_proc->pid);
                queue_add(ready_q,blocked_proc,blocked_proc->pid);
            }
            // otherwise error
            else { 
                TracePrintf(0,"\tHow did we get a delay clock of %d for process %d\n",blocked_proc->delay_clock,(int)blocked_proc->pid);
                queue_add(blocked_q,blocked_proc,blocked_proc->pid);
            }
        }
    }

    // check ready queue, if there are other processes ready
    size = queue_size(ready_q);
    TracePrintf(0,"\tReady queue size %d\n",size); 

    // if there are processes on ready queue
    if (size >= 1) {
        TracePrintf(0,"\tThere are processes in the ready queue\n");

        pcb_t *next_proc = queue_pop(ready_q);

        // check that we successfully got the process
        if ((curr_proc == NULL) || (next_proc == NULL)) {
            TracePrintf(0,"Error, queue_pop failed for either the running or ready queue\n");
        }

        // bookkeeping with queues, update running and ready
        queue_add(running_q, next_proc, next_proc->pid);
        queue_add(ready_q, curr_proc, curr_proc->pid);

        // copy user context into current process
        curr_proc->user_context = (UserContext *)ctx;

        // invoke KCSwitch
        int rc = KernelContextSwitch(KCSwitch,curr_proc,next_proc);

        // check return code
        if (rc != 0) {
            TracePrintf(0,"ERROR: Something went wrong with KernelContext when trying to go from processs %d to %d\n",(int)curr_proc->pid,(int)next_proc->pid);
        }

        // now that we're back, more bookkeeping with queues, change things back now that process A
        // is running
        queue_remove(running_q,next_proc->pid);
        queue_remove(ready_q,curr_proc->pid);

        queue_add(running_q, curr_proc, curr_proc->pid);
        queue_add(ready_q, next_proc, next_proc->pid);

        // restore ctx
        ctx = (void*)curr_proc->user_context;
    }
    // if no other processes ready, check if blocked, then do nothing
    else {
        TracePrintf(0,"\tNo other processes detected. Will check if running process is blocked\n");

        // check clock ticks of current process
        if (curr_proc->delay_clock < 0) {
            TracePrintf(0,"\tIt seems our only running process is blocked, decrementing...\n");
            curr_proc->delay_clock--;
        }
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

// ********************************************************** 
//                     Syscall Handlers
// **********************************************************

// ==========================================
// =    Basic process coordination 3.1.1    =
// ==========================================

/**
 * @brief 
 * 
 * @return int 
 */
int KernelFork(void) {
    // check global running queue
    // give error if it's empty

    // get current running process from the queue
    // make sure that everything's valid, give error if not

    // increment their number of kids
    // generate a pid for their child 
    // add pid to the parent's array of child pid's

    // initialize new child process, giving it the assigned pid
        // call KCCopy to copy kernel context into new process
        // initiialize child's page table 
        // initialize child's user context and address space
        // have child's parent variable point to parent process
    
    // set returncode to the child's pid
    
    // move parent process to ready queue
    // move child process to running queue

    // context switch to child
        // set returncode to 0   
    // give error if contextswitch failed

    // return returncode
}

/**
 * @brief 
 * 
 * @param filename 
 * @param argvec 
 * @return int 
 */
int KernelExec(char *filename, char **argvec) {
    // check arguments
    // give error if invalid, and do not destroy process

    
    // get argc by counting number of entries before first NULL of argvec
    // get argv by looking at strings pointed at by argvec
    // if anything goes wrong, return error

    // load new program indicated by filename into process's address space
        // from here onwards, we can't recover this process
    // don't change process, execute main(argc, argv) by referring to filename
    // if failure, kill this process and return error
    // if success, we don't return.
}

/**
 * @brief 
 * 
 * @param status 
 */
void KernelExit(int status) {
    // check global running queue
    // give error if its empty
    
    // if the current process is the initial process (find by checking pid)
        // halt system

    // get current running queue
    // for each child process
        // go to child processs and set their parent pointer to NULL
    // change its status to dead
    // free all other stuff in process
    // move process to defunct queue
    

}

/**
 * @brief 
 * 
 * @param status_ptr 
 * @return int 
 */
int KernelWait(int *status_ptr) {
    // give error if it's empty
    // 
    // check global ready, running and defunct queues for child proccesses
        // if a child process is in the running or defunct queue, block the parent.
            // might design blocks to handle differenct situations
        // if the child proccess is in the defunt queue, then get it's return code
    // probably need to check for children everytime a child process dies
 
    // Note: maybe will design the defunct queue in a way that if a proccess terminates and it's parent is still alive then it goes into the defunct queue
    // also vice-versa: if a process terminates and any of it's children is still alive then it becomes defunct
}

/**
 * @brief Get the Pid object
 * 
 * @return int 
 */
int KernelGetPid(void) {
    // check global running queue
    int size = queue_size(running_q);
    if (size != 1) {
        TracePrintf(0,"ERROR in GetPid, how do we have a running queue of size %d?\n",size);
    }
    
    // access running process via queue and return its pid
    pcb_t *curr_proc = queue_pop(running_q);

    // since we popped, we add it back
    queue_add(running_q,curr_proc,curr_proc->pid);

    return curr_proc->pid;

   
}

/**
 * @brief 
 * 
 * @param addr 
 * @return int 
 */
int KernelBrk(void *addr) {
    // NOTE: refer to kernelsetbrk
    // check global running queue
    int size = queue_size(running_q);
    if (size != 1) {
        TracePrintf(0,"ERROR in GetPid, how do we have a running queue of size %d?\n",size);
    }

    // access running process via queue
    pcb_t *curr_proc = queue_pop(running_q);
    // since we popped, we add it back to running queue
    queue_add(running_q,curr_proc,curr_proc->pid);

    // check if given address is valid, if invalid, give ERROR
    if (addr == NULL) {
        TracePrintf(0,"ERROR, we got an invalid brk addr, it's NULL!\n");
        return ERROR;
    }

    // index in page table of user brk
    int curr_heap_index = curr_proc->user_heap_pt_index;

    // page table index of base of region1 (for indexing purposes)
    int user_pt_base_index = VMEM_1_BASE >> PAGESHIFT;

    // when setting a new brk, we'll call up to page and put our brk there, so that we 
    // allocate memory for the heap 1 page at a time.

    // index in page table of address given, we'll just take up the entire page
    int addr_index = UP_TO_PAGE((int)addr >> PAGESHIFT - user_pt_base_index);
    // so addr_index is the potential new brk: the first invalid address (expressed as index in pt)

    TracePrintf(0,"Within KernelBrk(): our current heap index is at %d, our address is at index %d\n",curr_heap_index,addr_index);

    // if (location > addr and < sp are not in process address space)
    if ((addr_index > curr_proc->user_stack_pt_index) || (addr_index < curr_heap_index)) {
        TracePrintf(0,"The new brk address provided is either in the stack, or below our current brk\n");
        return ERROR;
    }

    // looping through each virtual page between current brk and proposed brk
    // we check the bit vector AND the user page table valid bit, if any of them
    // are not free, error.
    for (curr_heap_index ; curr_heap_index < addr_index ; curr_heap_index++) {
        if (bit_vector[curr_heap_index + user_pt_base_index] == PAGE_NOT_FREE) {
            TracePrintf(0,"ERROR, bit vector claims user pt index %d (which is at bit_vector index %d) is taken\n",curr_heap_index,curr_heap_index+user_pt_base_index);
            return ERROR;
        }
        else if (curr_proc->user_page_table[curr_heap_index].valid == PAGE_NOT_FREE) {
            TracePrintf(0,"ERROR, user page table index %d is not free\n",curr_heap_index);
            return ERROR;
        }
    }

    // if we made it here, it means we have space
    TracePrintf(0,"in KernelBrk(): we've got space, updating page tables and bit vector...\n");

    // loop through and allocate heap in pagetable
    for (curr_heap_index = curr_proc->user_heap_pt_index ; curr_heap_index < addr_index ; curr_heap_index++) {
        

        // update page table
        curr_proc->user_page_table[curr_heap_index].valid = VALID_FRAME;
        
        // set up pte
        // curr_proc->user_page_table[curr_heap_index].pfn = // find a 
        pte_t entry;
        entry.valid = VALID_FRAME;
        entry.prot = NO_X_W_R;
        entry.pfn = AllocatePFN();

        if (entry.pfn == -1) {
            TracePrintf(0,"ERROR, in AllocatePFN no free frames found!\n");
            Halt();
        }


        // update bit_vector
        bit_vector[entry.pfn] = PAGE_NOT_FREE;

        

        // u_pt[index - vp0] = entry;

    }
    TracePrintf(0,"Exiting KernelBrk()...\n");

    // return success
    return 0;
}


// ==============================
// =    I/O Syscalls 3.1.2      =
// ==============================

/**
 * @brief 
 * 
 * @param clock_ticks 
 * @return int 
 */
int KernelDelay(int clock_ticks) {

    if (clock_ticks == 0) {
        return 0;
    } else if (clock_ticks < 0) {
        return ERROR;
    } 

    // check global running queue
    int size = queue_size(running_q);
    if (size != 1) {
        TracePrintf(0,"ERROR in GetPid, how do we have a running queue of size %d?\n",size);
    }

    // access running process via queue
    pcb_t *curr_proc = queue_pop(running_q);
    // since we popped, we add it back to running queue
    queue_add(running_q,curr_proc,curr_proc->pid);

    curr_proc->delay_clock = clock_ticks;

    // move current process to blocked process queue
    
    



    // remove process from blocked queue and onto ready queue.

    return 0; 

}

/**
 * @brief 
 * 
 * @param tty_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelTtyRead(int tty_id, void *buf, int len) {
    // pseudocode
}

// ================================================
// =    InterProcess Communication (IPC) 3.1.3    =
// ================================================

/**
 * @brief 
 * 
 * @param pipe_idp 
 * @return int 
 */
int KernelPipeInit(int *pipe_idp) {
    // return error if address null

    // allocate p_max bytes in kernel memory

    // generate pipe id by looking at other existing pipes
    // initialize pipe with its assigned id
    // save id at *pipe_idp

    // return error if anything failed (e.g. no more memory)

    // return 0
}

/**
 * @brief 
 * 
 * @param pipe_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelPipeRead(int pipe_id, void *buf, int len) {
    // check arguments are valid (no negatives or nulls)
        // error if anything invalid

    // check ids of pipes
    // if id matches

    // get plen, current length of data in pipe

    // if pipe is empty
        // block caller
    // if len < plen
        // put len data in buf, and remove len data from pipe
        // return len
    // if len > plen
        // put plen data into buf,remove plen data from pipe
        // return plen

    // return error if any of the pipe manipulating functions fail
}

/**
 * @brief 
 * 
 * @param pipe_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelPipeWrite(int pipe_id, void *buf, int len) {
    // check arguments are valid (no negatives or nulls)
        // error if anything invalid

    // check ids of pipes
    // if id matches

    // get plen, current length of data in pipe

    // int available space = max pipe length - plen

    // if available space >= len
        // put len data from buf into pipe
        // signal any readers
        // return len
    
    // else if available space < len
        // put available space data from buf into pipe
        // signal any readers
        // return available space

    // return error if we get here
}

// ==========================================
// =    Synchronization Syscalls 3.1.4      =
// ==========================================

// What does he mean by synchronization calls operating on processes vs pthreads???

/**
 * @brief 
 * 
 * @param lock_idp 
 * @return int 
 */
int KernelLockInit(int *lock_idp) {
    // check for a valid argument

    /* This was at page 49 of textbook, not sure if this is right for yalnix?? */
    // allocate and initialize per-thread data structures? In this simple yalnix context will that just be allocing new processes?
    // put the thread (process?) in the ready state by adding it to a ready list/ queue. Use queue adding function
    /*********/

    // creates a lock handle lock_idp

    // save identifier at *lock_idp

    // Return error if any
}

/**
 * @brief 
 * 
 * @param lock_id 
 * @return int 
 */
int KernelAcquire(int lock_id) {

    // block caller by adding it to blocked processes queue.


    //continue once this process is out of blocked processes queue and in running queue.

    // return if failure. 
}

/**
 * @brief 
 * 
 * @param lock_id 
 * @return int 
 */
int KernelRelease(int lock_id) {
    // check if caller has lock
	// return error if they doesn't
    
    // Release lock that was being waited upon by processes in blocked queue.
	
    // return error or not. 
}

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @return int 
 */
int KernelCvarInit(int *cvar_idp) {
    // init a cvar and assign to cvar_idp.
    // return error or not
}

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @return int 
 */
int KernelCvarSignal(int cvar_idp) {
    // while (1)
	// let waiter run by adding process to blocked processes queue

	
    // return error or not
}

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @return int 
 */
int KernelCvarBroadcast(int cvar_idp) {
    // while (1)
	// release all processes in blocked processes queue. 

    // return error or not
}

/**
 * @brief 
 * 
 * @param cvar_idp 
 * @param lock_id 
 * @return int 
 */
int KernelCvarWait(int cvar_idp, int lock_id) {
    // block the current process by adding it to the blocked queue. 
    // release the the mutex lock signaled by lock_idp
    
    // while (1)
	// wait for signal identified by cvar_id 
	// reaqure lock
    // once the lock is aquired then continue. DO NOT RETURN UNTIL LOCK IS REAQUIRED

    // return error code or no error
}

/**
 * @brief 
 * 
 * @param id 
 * @return int 
 */
int KernelReclaim(int id) {
    // destory the lock identified by id, if there is one.
    // release associated resources, if any. 

    // return success or not.
}
