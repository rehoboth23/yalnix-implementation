#include <hardware.h>
#include <ykernel.h>
#include <yuser.h>
#include <ylib.h>
#include "kernel.h"
#include "process.h"

// ********************************************************** 
//                     Syscall Handlers
// **********************************************************

// ==========================================
// =    Basic process coordination 3.1.1    =
// ==========================================

/**
 * @brief 
 * 
 * @param uctxt 
 * @return int 
 */
int KernelFork(UserContext *uctxt) {
    // check for null user context
    if (uctxt == NULL) {
        TracePrintf(0,"ERROR: KernelFork got a null user context\n");
        return ERROR;
    }

    // initialize child PCB
    pcb_t *childPCB = init_process(uctxt);

    // error if init_process failed
    if (childPCB == NULL) {
        TracePrintf(0,"ERROR: child PCB in KernelFork is null.\n");
        return ERROR;
    }

    pte_t *k_pt = ( pte_t *) ReadRegister(REG_PTBR0);  // current kernel stack should be region 0 stack
    int kernel_stack_base = (int) KERNEL_STACK_BASE >> PAGESHIFT;
    int kernel_base = (int) VMEM_0_BASE >> PAGESHIFT;

    // find an invalid frame to be reserved for kernel
    int reserved_kernel_index = -1;
    for (int index =kernel_stack_base ; index >= kernel_base; index--) {
        if (k_pt[index].valid == INVALID_FRAME) {
            reserved_kernel_index = index;
            break;
        }
    }
    // error if we can't find any
    if (reserved_kernel_index == -1) {
        return ERROR;
    }

    // update that reserved frame
    k_pt[reserved_kernel_index].valid = VALID_FRAME;
    k_pt[reserved_kernel_index].prot = NO_X_W_R;
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    // copy user page table
    if (CopyUPT(activePCB->user_page_table, childPCB->user_page_table, k_pt, reserved_kernel_index) == ERROR) {
        TracePrintf(0,"ERROR: KernelFork, failed to CopyUPT\n");
        return ERROR;
    }

    // free kernel index when done (make invalid)
    k_pt[reserved_kernel_index].pfn = 0;
    k_pt[reserved_kernel_index].valid = INVALID_FRAME;
    k_pt[reserved_kernel_index].prot = NO_X_NO_W_NO_R;

    // add the childPCB to the ready queue
    if (queue_add(ready_q, childPCB, childPCB->pid) == ERROR) {
        TracePrintf(0,"ERROR: KernelFork, failed add to queue.\n");
        return ERROR;
    }

    // context switch to the child
    KernelContextSwitch(KCCopy, childPCB, NULL);
    // once back from context switch

    // return 0 if child
    if (activePCB->pid == childPCB->pid) return 0;

    TracePrintf(0, "Number of children -> %d\n", activePCB->num_children);
    // return childpid if parent
    return childPCB->pid;
}

/**
 * @brief 
 * 
 * @param uctxt 
 * @param filename 
 * @param argvec 
 * @return int 
 */
int KernelExec(UserContext *uctxt, char *filename, char **argvec) {
    if (uctxt == NULL || filename == NULL || argvec == NULL){
        TracePrintf(0,"ERROR: KernelExec received a NULL argument\n");
        return ERROR;
    } 
    // reset the user context
    memset(&(activePCB->user_context), 0, sizeof(UserContext));

    // flush the kernel stack tlb
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);

    // load the new program into the pcb
    if (LoadProgram(filename, argvec, activePCB) == ERROR) {
        TracePrintf(0,"ERROR: KernelExec failed to loadprogram.\n");
        return ERROR;
    }
    return 0;
}

/**
 * @brief 
 * 
 * @param uctxt 
 * @param exit_code 
 */
int KernelExit(int exit_code, UserContext *uctxt) {

    // error check
    if(uctxt == NULL) {
        TracePrintf(0,"ERROR: KernelExit received a NULL argument\n");
        return ERROR;
    }


    TracePrintf(0, "Ready -> %d ::: Blocked -> %d ::: Defunct -> %d\n", ready_q->size, blocked_q->size, defunct_q->size);
    
    // update exit_code in PCB
    activePCB->exit_code = exit_code;
    int limit;
    queue_t *swap_q = NULL; // queue to swap process with
    if (activePCB->ppid != 0 || activePCB->num_children > 0) {

    // update parents && children in the ready queue
        limit = ready_q->size;

        // looping through ready queue
        for (int i = 0; i < limit; i++) {
            pcb_t *r_pcb = queue_pop(ready_q);

            // if the pcb we popped is the child of our activePCB
            if (r_pcb->ppid == activePCB->pid) {
                // create orphan :(
                r_pcb->ppid = 0;
            } 
            // if the pcb we popped is the parent of our active pcb
            else if (r_pcb->pid == activePCB->ppid) {
                // update the popped pcb's number of children
                TracePrintf(0, "~~~ Children left -> %d children\n", r_pcb->num_children);
                swap_q = defunct_q;
            }
            if (queue_add(ready_q, r_pcb, r_pcb->pid) == ERROR) {
                TracePrintf(0,"ERROR: KernelExit, unable to add to queue in ready q for loop\n");
                return ERROR;
            }
        }

    // update parents && children in the blocked queue

        limit = blocked_q->size;
        // loop through the blocked queue
        for (int i = 0; i < limit; i++) {
            pcb_t *b_pcb = queue_pop(blocked_q);

            // error checking
            if (b_pcb == NULL) {
                TracePrintf(0,"ERROR: KernelExit, b_pcb from queue is null.\n");
                return ERROR;
            }

            // if the pcb we popped is the parent of our active process and it's blocked
            if (b_pcb->pid == activePCB->ppid && b_pcb->blocked_code == BLOCKED_WAIT) {
                // update exit code of active pcb
                b_pcb->blocked_code = activePCB->exit_code;
                b_pcb->num_children--;
                TracePrintf(0, "~~~ Children left -> %d children\n", b_pcb->num_children);
                if (queue_add(ready_q, b_pcb, b_pcb->pid) == ERROR) {
                    TracePrintf(0,"ERROR: KernelExit, unable to add to queue in blocked q for loop\n");
                    return ERROR;
                }
                continue;
            } 
            // if the pcb we popped is the parent and is not blocked
            else if (b_pcb->pid == activePCB->ppid) {
                TracePrintf(0, "~~~ Children left -> %d children\n", b_pcb->num_children);
                swap_q = defunct_q;
            } 
            // if the pcb we popped is the child of the active process
            else if (b_pcb->ppid == activePCB->pid) {
                // make it an orphan
                b_pcb->ppid = 0;
            }
            if (queue_add(blocked_q, b_pcb, b_pcb->pid) == ERROR) {
                TracePrintf(0,"ERROR: KernelExit, unable to add to queue in blocked q for loop\n");
                return ERROR;
            }
        }

        // if any children are in the defunct queue take them out
        limit = defunct_q->size;
        for (int i = 0; i < limit; i++) {
            // for now only condition for defunct is if it has a ready/blocked parent
            pcb_t *d_pcb = queue_pop(defunct_q);

            if (d_pcb == NULL) {
                TracePrintf(0,"ERROR: KernelExit, d_pcb from queue is null.\n");
                return ERROR;
            }

            if (d_pcb->ppid != activePCB->pid) {
                if (queue_add(defunct_q, d_pcb, d_pcb->pid) == ERROR) {
                    TracePrintf(0,"ERROR: KernelExit, unable to add to defunct queue.\n");
                    return ERROR;
                }
            }
        }
    }

    // if nothing to swap
    if (swap_q == NULL) {
        if (delete_process(activePCB) == ERROR) {
            TracePrintf(0,"ERROR: KernelExit, unable to delete process.\n");
            return ERROR;
        }
    } 
    
    // otherwise
    else {
         if (free_addr_space(activePCB->user_page_table, activePCB->kernel_stack_pt) == ERROR) {
            TracePrintf(0,"ERROR: KernelExit, unable to delete process.\n");
            return ERROR;
        }
    }

    // then swap process
    if (SwapProcess(swap_q,uctxt) == ERROR) {
        TracePrintf(0, "ERROR: KernelExit, Unable to swap process.\n");
        return ERROR;
    }
    TracePrintf(0, "Ready -> %d ::: Blocked -> %d ::: Defunct -> %d\n", ready_q->size, blocked_q->size, defunct_q->size);

    return 0;
}

/**
 * @brief 
 * 
 * @param status_ptr 
 * @return int
 */
int KernelWait(int *status_ptr, UserContext *uctxt) {
    // check status_ptr and num of children, it shouldn't be null or 0
    if (status_ptr == NULL || activePCB->num_children == 0) {
        *status_ptr = ERROR;
        return ERROR;           // so calling wait with no children will just give ERROR
    }

    // check defunct queue
    int limit = defunct_q->size;
    for(int i = 0; i < limit; i++) {
        // pop a queue out of defunct_q
        pcb_t *d_pcb = queue_pop(defunct_q);

        if (d_pcb == NULL) {
            TracePrintf(0, "ERROR: KernelWait, Unable to pop from queue.\n");
            return ERROR;
        }

        // if dead pcb is the child of active pcb
        if (d_pcb->ppid == activePCB->pid) {
            activePCB->num_children--;
            *status_ptr = d_pcb->exit_code; // dead pcb still has exit code saved
            return 0;
        }

        // add dead pcb back to defunct q
        if(queue_add(defunct_q, d_pcb, d_pcb->pid) == ERROR) {
            TracePrintf(0, "ERROR: KernelWait, unable to add to queue.\n");
            return ERROR;
        }
    }
    // mark active process as blocked
    activePCB->blocked_code = BLOCKED_WAIT;
    //  save pid for when we come back
    int set_code_pid = activePCB->pid;

    // swap process
    if (SwapProcess(blocked_q, uctxt) == ERROR) {
        TracePrintf(0, "ERROR: KernelWait, unable to swap process.\n");
        return ERROR;
    }

    // unblock once we swap back
    if (activePCB->pid == set_code_pid) {
        *status_ptr = activePCB->blocked_code;
        activePCB->blocked_code = NOT_BLOCKED;
    }
    return 0;
}

/**
 * @brief Get the Pid of process
 * 
 * @return int 
 */
int KernelGetPid(void) {
    if (activePCB == NULL) {
        return ERROR;
    }
    return activePCB->pid;
}

/**
 * @brief Get the PPid of process
 * 
 * @return int 
 */
int KernelGetPPid(void) {
    if (activePCB == NULL) {
        return ERROR;
    }
    return activePCB->ppid;
}

/**
 * @brief 
 * 
 * @param addr 
 * @return int 
 */
int KernelBrk(void *addr) {
    if (addr == NULL) {
        TracePrintf(0, "Trying to set NULL brk!\n");
        return ERROR;
    }

    // get indexes for each segment
    int user_page_base = VMEM_1_BASE >> PAGESHIFT;
    int heap_index = activePCB->user_heap_pt_index;
    int stack_index = activePCB->user_stack_pt_index;
    int data_index = activePCB->user_data_pt_index;

    // page of the given address
    int target = ( UP_TO_PAGE( (int)addr) >> PAGESHIFT ) - user_page_base;

    // index in page table of address given, we'll just take up the entire page
    if (target >= stack_index || target <= data_index) {
        TracePrintf(0, "INVALID ADDRESS ::: target -> %d heap_index -> %d data_index -> %d stack_index -> %d\n", target, heap_index, data_index, stack_index);
        return ERROR;
    }

    TracePrintf(0, "KernelBRK called ::: addr_index -> %d heap_index -> %d data_index -> %d stack_index -> %d\n", target, heap_index, data_index, stack_index);
    
    // if we're growing our heap
    if ( target > heap_index) {
        // first loop, ensure that all virtual frames between old br and new brk are free
        for (int i = heap_index; i < target; i++) {
            if (activePCB->user_page_table[i].valid == VALID_FRAME) return ERROR;
        }
        // second loop, allocate virtual frames
        for (int i = heap_index; i < target; i++) {
            if (activePCB->user_page_table[i].valid == INVALID_FRAME) {
                TracePrintf(0, "Brk: index -> %d\n", i);
                // find any pfn
                int pfn = AllocatePFN();
                if (pfn == ERROR) {
                    return ERROR;
                    free_addr_space(activePCB->user_page_table, activePCB->kernel_stack_pt); // Talk to KC about this line here...
                }
                // set up page table entry permissions
                activePCB->user_page_table[i].prot = NO_X_W_R;
                activePCB->user_page_table[i].pfn = pfn;
                activePCB->user_page_table[i].valid = VALID_FRAME;
            }
        }
    } 

    // otherwise, we're shrinking our heap
    else {
        // first loop (a sanity check), ensure that our heap is marked as taken
        // if some part of our heap was free, something went wrong
        for (int i = target; i < heap_index; i++) {
             if (activePCB->user_page_table[i].valid == INVALID_FRAME) return ERROR;
        }
        // second loop, free the virtual frames
        for (int i = target; i < heap_index; i++) {
            if (activePCB->user_page_table[i].valid == VALID_FRAME) {
                TracePrintf(0, "index -> %d\n", i);
                // update pfn_list
                if (DeallocatePFN(activePCB->user_page_table[i].pfn) == ERROR) {
                    TracePrintf(0, "ERROR: KernelBrk, unable to deallocate PFB.\n");
                    return ERROR;
                }

                // set up page table entry
                activePCB->user_page_table[i].prot = 0;
                activePCB->user_page_table[i].pfn = 0;
                activePCB->user_page_table[i].valid = INVALID_FRAME;
            }
        }
    }

    activePCB->user_heap_pt_index = target;
    return 0;
}

// ==============================
// =    I/O Syscalls 3.1.2      =
// ==============================

/**
 * @brief 
 * 
 * @param uctxt 
 * @param clock_ticks 
 * @return int 
 */
int KernelDelay(int clock_ticks, UserContext *uctxt) {

    if (clock_ticks == 0) {
        return 0;
    } else if (clock_ticks < 0) {
        return ERROR;
    } 

    // delay assumes that the idle process will never be blocked ensuring that we always have an available proccess

    // some bookkeeping
    activePCB->clock_ticks = clock_ticks;
    activePCB->blocked_code = BLOCKED_DELAY;

    // swap process
    if (SwapProcess(blocked_q,uctxt) == ERROR) {
        TracePrintf(0, "ERROR: KernelDelay, unable to swap processes.\n");
        return ERROR;
    }

    return 0; 

}

/**
 * @brief 
 * 
 * @param uctxt
 * @param tty_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelTtyRead(UserContext *uctxt, int tty_id, void *buf, int len) {
    char *ttyBuffer = ttyReadbuffers[tty_id];
    queue_t *ttyQueue = ttyReadQueues[tty_id];

    if (ttyQueue == NULL || ttyBuffer == NULL) {
        // some error stuff
        return ERROR;
    } else if (len < 0) {
        // some error stuff
        return ERROR;
    } else if (len == 0) return 0; // nothing to read

    // verify that there are no processes waiting to read from the terminal and there is input to read
    if (ttyQueue->size > 0 || ttyBuffer[0] == 0) {
        activePCB->blocked_code = BLOCKED_TTY_READ;
        activePCB->tty_terminal = tty_id;
        SwapProcess(ttyQueue, uctxt);
    }

    int bytes_num;

    // looping up until the first new line   
    for(bytes_num = 0; bytes_num < ttyReadTrackers[tty_id]; bytes_num++) {

        // if we see a new line
        if(ttyBuffer[bytes_num] == '\n') {
            bytes_num++;
            break;
        }
    }

    if (bytes_num > len) {
        bytes_num = len;
    }

    //copy the bytes  (as many as possible) into buf
    memcpy(buf, ttyBuffer, bytes_num);

    // temporary buffer to use to move the bytes up
    char tempBuffer[ttyReadTrackers[tty_id] - bytes_num];

    // save the remaining bytes in terminal buffer to a temporary buffer
    memcpy(tempBuffer, &(ttyBuffer[bytes_num]), ttyReadTrackers[tty_id] - bytes_num);

    // clear the tty buffer for the terminal
    memset(ttyBuffer, 0, ttyReadTrackers[tty_id]);

    // move the save bytes bytes to the terminal buffer
    memcpy(ttyBuffer, tempBuffer, ttyReadTrackers[tty_id] - bytes_num);
    ttyReadTrackers[tty_id] = ttyReadTrackers[tty_id] - bytes_num;
    TracePrintf(0, "KernelTtyRead LOG: track %d\n", ttyReadTrackers[tty_id]);

    // move next reader (if any) to the ready queue
    if (ttyQueue->size > 0 && ttyBuffer[0] != 0) {
        pcb_t *nextReader = queue_peek(ttyQueue);
        nextReader->blocked_code = NOT_BLOCKED;
        queue_add(ready_q, nextReader, nextReader->pid);
    }

    return bytes_num;
}

/**
 * @brief 
 * 
 * @param uctxt
 * @param tty_id 
 * @param buf 
 * @param len 
 * @return int 
 */
int KernelTtyWrite(UserContext *uctxt, int tty_id, void *buf, int len) {
    
    // have a queue for processes waiting to write
    queue_t *ttyQueue = ttyWriteQueues[tty_id];
	
	// error checking
    if (ttyQueue == NULL) {
        // some error stuff
        return ERROR;
    } else if (len < 0) {
        // some error stuff
        return ERROR;
    } else if (len == 0) return 0; // nothing to write
	
	// add calling prrocess
    queue_add(ttyQueue, activePCB, activePCB->pid);

    // verify that there are no processes waiting to write to the terminal and there is output to write
    if (queue_peek(ttyQueue)->pid != activePCB->pid) {
        activePCB->blocked_code = BLOCKED_TTY_WRITE;
        activePCB->tty_terminal = tty_id;
        
        // swapping process while keeping the current process at the top of its queue, because it has more writing to do. This is to prevent another process from writing to the the same terminal before this process is done
        SwapProcess(NULL, uctxt);
    }
    
    int bytes_left = len;
    int bytes_written = 0;
    
    // while there's more to write
    while (bytes_left > 0) {
    
    	// while the terminal is taken
        while (ttyWriteTrackers[tty_id] == TERMINAL_CLOSED) {
        
        	// mark the current process as blocked
            activePCB->blocked_code = BLOCKED_TTY_TRANSMIT;
            activePCB->tty_terminal = tty_id;
            
            // swap process without putting anyone else in front, so we're still in front of the queue
            SwapProcess(NULL, uctxt);
        }
        
        // mark the terminal as taken
        ttyWriteTrackers[tty_id] = TERMINAL_CLOSED;

        int to_write;

        if (bytes_left > TERMINAL_MAX_LINE) {
            to_write = TERMINAL_MAX_LINE;
        } else {
            to_write = bytes_left;
        }
        
        // buffer to write to
        char *ttyBuffer[to_write];
        
        // copy truncated info bytes to buffer
        memcpy(ttyBuffer, buf + bytes_written, to_write);
        
   		// ttytransmit
        TtyTransmit(tty_id, ttyBuffer, to_write);
        
        // update how much we've written and how much there is left to write
        bytes_left -= to_write;
        bytes_written += to_write;
    }

	// wake up anyone waiting to write to the same terminal
    queue_pop(ttyQueue);
    if (ttyQueue->size > 0) {
        pcb_t *nextWriter = queue_peek(ttyQueue);
        nextWriter->blocked_code = NOT_BLOCKED;
        queue_add(ready_q, nextWriter, nextWriter->pid);
    }

    return bytes_written;
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
