/*
 *  traphandlers.c
 *  
 *  Defines function handlers for all traps/syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#include "kernel.h"
#include "pipe.h"
#include "interrupt.h"

// ********************************************************** 
//                      Trap Handlers
// **********************************************************

/*
 * TrapKernelHandler
 *
 * Handler in interrupt vector table for TRAP_KERNEL
 */
void TrapKernelHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    switch (user_context->code) {
         case YALNIX_FORK: 
            TracePrintf(0, "kernel calling Fork()");
            break;
        case YALNIX_EXEC:
            TracePrintf(0, "kernel calling Exec()");
            break;
        case YALNIX_EXIT:
            TracePrintf(0, "kernel calling Exir()");
            break;
        case YALNIX_WAIT:
            TracePrintf(0, "kernel calling Wait()");
            break;
        case YALNIX_GETPID:
            TracePrintf(0, "kernel calling GetPid()");
            break;
        case YALNIX_BRK:
            TracePrintf(0, "kernel calling Brk()");
            break;
        case YALNIX_DELAY:
            TracePrintf(0, "kernel calling Delay()");
            break;
        case YALNIX_TTY_READ:
            TracePrintf(0, "kernel calling TtyRead()");
            break;
        case YALNIX_TTY_WRITE:
            TracePrintf(0, "kernel calling TtyWrite()");
            break;
        default:
            TracePrintf(0, "Unknown code");
            break;
    }
    // check code of user_context

    // depending on code, call the corresponding function below

    // arguments are in regs[] in usercontext, starting with regs[0]

    // return arguments should be in regs[0]
}

/*
 * TrapClockHandler
 *
 * Handler in interrupt vector table for TRAP_CLOCK
 *  
 */
void TrapClockHandler(void *ctx) {
    // check ready queue, if there are other processes
    // call context switch on them

    // if no other processes ready, dispatch idle
}

/*
 * TrapIllegalHandler
 *
 * Handler in interrupt vector table for TRAP_ILLEGAL
 *  
 */
void TrapIllegalHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // same as TrapMathHandler
}

/*
 * TrapMemoryHandler
 *
 * Handler in interrupt vector table for TRAP_MEMORY
 *  
 */
void TrapMemoryHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // make sure user context is valid
    
    // chcek if this is an implicit request
    // to enlarge memory on process's stack
            // question: how to check if implicit req?
    // if so: enlarge stack to cover the address in the addr
    // field os UserContext, then return

    // otherwise, same as TrapMathHandler
}

/*
 * TrapMathHandler
 *
 * Handler in interrupt vector table for TRAP_MATH
 *  
 */
void TrapMathHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // get current running process from running queue
    // check that it's valid

    // abort it
    // get another ready process to run (or do we call some function to do this?)
}

/*
 * TrapReceiveHanlder
 *
 * Handler in interrupt vector table for TRAP_TTY_RECEIVE
 *  
 */
void TrapTTYReceiveHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // check that usercontext is valid

    // code of usercontext is the terminal has a new line
    // call TtyReceive from hardware.h to read input from terminal
    
    // buffer input line for more user TtyRead syscalls if necessary
    
}
/*
 * TrapTransmitHandler
 *
 * Handler in interrupt vector table for TRAP_TTY_TRANSMIT
 *  
 */
void TrapTTYTransmitHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // check that usercontext is valid

    // code field of usercontext is the terminal that completed
    //a TtyTransmit operation

    // if process that started terminal output is blocked, finish it
    // start the next terminal output on this terminal if there are any

}

/*
 * TrapDiskHandler
 *
 * Handler in interrupt vector table for TRAP_DISK
 *  
 */
void TrapDiskHandler(void *ctx) {
    UserContext *user_context = (UserContext *) ctx;
    // no need to worry about, this is extra functionality
    // to do with disk
}

// ********************************************************** 
//                     Syscall Handlers
// **********************************************************

// ==========================================
// =    Basic process coordination 3.1.1    =
// ==========================================


int Fork(void) {
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

int Exec(char *filename, char **argvec) {
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

void Exit(int status) {
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

int Wait(int *status_ptr) {
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

int GetPid(void) {
    // check global running queue
    // give error if it's empty

    // access running process via queue and return its pid
}
int Brk(void *addr) {
    // NOTE: refer to kernelsetbrk
    // check global running queue
    // give error if its empty

    // set PAGEOFFSET to addr, which is rounded up to the multiple of PAGESIZE using UP_TO_PAGE or DOWN_TO_PAGE macro

    // if (location > addr and < sp are not in process address space)
        //return ERROR

    // return ERROR if not enough memory is avaible or addr is invalid

    // return success
}


// ==============================
// =    I/O Syscalls 3.1.2      =
// ==============================

int Delay(int clock_ticks) {

    if (clock_ticks == 0) {
        return 0;
    } else if (clock_ticks < 0) {
        return ERROR;
    } 

    // move current process to blocked process queue

    //for (int i = 0; i < clock_ticks; i++) {
        // use some function to wait for a bit
    //}

    // OR, find something that does this
    // functionthatwaits(clock_ticks)

    // remove process from blocked queue and onto ready queue.

    return 0; 

}

int TtyRead(int tty_id, void *buf, int len) {
    // pseudocode
}

// ================================================
// =    InterProcess Communication (IPC) 3.1.3    =
// ================================================

int PipeInit(int *pipe_idp) {
    // return error if address null

    // allocate p_max bytes in kernel memory

    // generate pipe id by looking at other existing pipes
    // initialize pipe with its assigned id
    // save id at *pipe_idp

    // return error if anything failed (e.g. no more memory)

    // return 0
}

int PipeRead(int pipe_id, void *buf, int len) {
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

int PipeWrite(int pipe_id, void *buf, int len) {
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

int LockInit(int *lock_idp) {
    // check for a valid argument

    /* This was at page 49 of textbook, not sure if this is right for yalnix?? */
    // allocate and initialize per-thread data structures? In this simple yalnix context will that just be allocing new processes?
    // put the thread (process?) in the ready state by adding it to a ready list/ queue. Use queue adding function
    /*********/

    // creates a lock handle lock_idp

    // save identifier at *lock_idp

    // Return error if any
}

int Acquire(int lock_id) {

    // block caller by adding it to blocked processes queue.


    //continue once this process is out of blocked processes queue and in running queue.

    // return if failure. 
}

int Release(int lock_id) {
    // check if caller has lock
	// return error if they doesn't
    
    // Release lock that was being waited upon by processes in blocked queue.
	
    // return error or not. 
}

int CvarInit(int *cvar_idp) {
    // init a cvar and assign to cvar_idp.
    // return error or not
}

int CvarSignal(int cvar_idp) {
    // while (1)
	// let waiter run by adding process to blocked processes queue

	
    // return error or not
}

int CvarBroadcast(int cvar_idp) {
    // while (1)
	// release all processes in blocked processes queue. 

    // return error or not
}
int CvarWait(int cvar_idp, int lock_id) {
    // block the current process by adding it to the blocked queue. 
    // release the the mutex lock signaled by lock_idp
    
    // while (1)
	// wait for signal identified by cvar_id 
	// reaqure lock
    // once the lock is aquired then continue. DO NOT RETURN UNTIL LOCK IS REAQUIRED

    // return error code or no error
}
int Reclaim(int id) {
    // destory the lock identified by id, if there is one.
    // release associated resources, if any. 

    // return success or not.
}
