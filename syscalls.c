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
    if (uctxt == NULL) {
        TracePrintf(0,"ERROR: KernelFork got a null user context\n");
        return ERROR;
    }

    pcb_t *childPCB = init_process(uctxt);

    if (childPCB == NULL) {
        TracePrintf(0,"ERROR: child PCB in KernelFork is null.\n");
        return ERROR;
    }

    pte_t *k_pt = ( pte_t *) ReadRegister(REG_PTBR0);  // current kernel stack should be regioin 0 stack
    int kernel_stack_base = (int) KERNEL_STACK_BASE >> PAGESHIFT;
    int kernel_base = (int) VMEM_0_BASE >> PAGESHIFT;

    int reserved_kernel_index = -1;
    for (int index =kernel_stack_base ; index >= kernel_base; index--) {
        if (k_pt[index].valid == INVALID_FRAME) {
            reserved_kernel_index = index;
            break;
        }
    }
    if (reserved_kernel_index == -1) {
        return ERROR;
    }

    k_pt[reserved_kernel_index].valid = VALID_FRAME;
    k_pt[reserved_kernel_index].prot = NO_X_W_R;
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    if (CopyUPT(activePCB->user_page_table, childPCB->user_page_table, k_pt, reserved_kernel_index) == ERROR) {
        TracePrintf(0,"ERROR: KernelFork, failed to CopyUPT\n");
        return ERROR;
    }

    if (free_addr_space(childPCB->user_page_table, childPCB->kernel_stack_pt) == ERROR) {
        TracePrintf(0,"ERROR: KernelFork, failed to free address space\n");
        return ERROR;
    }

    // free kernel index when done (make invalid)
    k_pt[reserved_kernel_index].pfn = 0;
    k_pt[reserved_kernel_index].valid = INVALID_FRAME;
    k_pt[reserved_kernel_index].prot = NO_X_NO_W_NO_R;

    if (queue_add(ready_q, childPCB, childPCB->pid) == ERROR) {
        TracePrintf(0,"ERROR: KernelFork, failed add to queue.\n");
        return ERROR;
    }

    KernelContextSwitch(KCCopy, childPCB, NULL);
    if (activePCB->pid == childPCB->pid) return 0;

    TracePrintf(0, "Number of children -> %d\n", activePCB->num_children);
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

    if(uctxt == NULL) {
        TracePrintf(0,"ERROR: KernelExit received a NULL argument\n");
        return ERROR;
    }


    TracePrintf(0, "Ready -> %d ::: Blocked -> %d ::: Defunct -> %d\n", ready_q->size, blocked_q->size, defunct_q->size);
    activePCB->exit_code = exit_code;
    int limit;
    queue_t *swap_q = NULL;
    if (activePCB->ppid != 0 || activePCB->num_children > 0) {

        // update parents && children in the ready queue
        limit = ready_q->size;
        for (int i = 0; i < limit; i++) {
            pcb_t *r_pcb = queue_pop(ready_q);
            if (r_pcb->ppid == activePCB->pid) {
                r_pcb->ppid = 0;
            } else if (r_pcb->pid == activePCB->ppid) {
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
        for (int i = 0; i < limit; i++) {
            pcb_t *b_pcb = queue_pop(blocked_q);

            if (b_pcb == NULL) {
                TracePrintf(0,"ERROR: KernelExit, b_pcb from queue is null.\n");
                return ERROR;
            }

            if (b_pcb->pid == activePCB->ppid && b_pcb->blocked_code == BLOCKED_WAIT) {
                b_pcb->blocked_code = activePCB->exit_code;
                b_pcb->num_children--;
                TracePrintf(0, "~~~ Children left -> %d children\n", b_pcb->num_children);
                if (queue_add(ready_q, b_pcb, b_pcb->pid) == ERROR) {
                    TracePrintf(0,"ERROR: KernelExit, unable to add to queue in blocked q for loop\n");
                    return ERROR;
                }
                continue;
            } else if (b_pcb->pid == activePCB->ppid) {
                b_pcb->num_children--;
                TracePrintf(0, "~~~ Children left -> %d children\n", b_pcb->num_children);
                swap_q = defunct_q;
            } else if (b_pcb->ppid == activePCB->pid) {
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

    if (swap_q == NULL) {
        if (delete_process(activePCB) == ERROR) {
            TracePrintf(0,"ERROR: KernelExit, unable to delete process.\n");
            return ERROR;
        }
    } else {
         if (free_addr_space(activePCB->user_page_table, activePCB->kernel_stack_pt) == ERROR) {
            TracePrintf(0,"ERROR: KernelExit, unable to delete process.\n");
            return ERROR;
        }
    }

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
    if (status_ptr == NULL || activePCB->num_children == 0) {
        *status_ptr = ERROR;
        return ERROR;
    }
    int limit = defunct_q->size;
    for(int i = 0; i < limit; i++) {
        pcb_t *d_pcb = queue_pop(defunct_q);

        if (d_pcb) {
            TracePrintf(0, "ERROR: KernelWait, Unable to pop from queue.\n");
            return ERROR;
        }

        if (d_pcb->ppid == activePCB->pid) {
            activePCB->num_children--;
            *status_ptr = d_pcb->exit_code;
            return 0;
        }
        if(queue_add(defunct_q, d_pcb, d_pcb->pid) == ERROR) {
            TracePrintf(0, "ERROR: KernelWait, unable to add to queue.\n");
            return ERROR;
        }
    }
    
    activePCB->blocked_code = BLOCKED_WAIT;
    int set_code_pid = activePCB->pid;
    if (SwapProcess(blocked_q, uctxt) == ERROR) {
        TracePrintf(0, "ERROR: KernelWait, unable to swap process.\n");
        return ERROR;
    }

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

    activePCB->clock_ticks = clock_ticks;
    activePCB->blocked_code = BLOCKED_DELAY;
    if (SwapProcess(blocked_q,uctxt) == ERROR) {
        TracePrintf(0, "ERROR: KernelDelay, unable to swap processes.\n");
        return ERROR;
    }

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
