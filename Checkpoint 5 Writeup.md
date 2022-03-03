# Checkpoint 5 Writeup

**Did we make the checkpoint?**







So userland code calls `TtyWrite`

- kernel blocks caller and dispatches someone else until terminal is free to write to
  - so what indicates that the terminal wasn't free?
- we invoke `TtyTransmit` when terminal is ready
- hardware throws `TRAP_TTY_TRANSMIT` when done
  - we move the original caller back to ready queue



TtyTransmit and TtyReceive are helper functions, the `buf` within it needs to be in region 0



## Implementation



### Blocked Codes

There are lots of ways for a process to get blocked, `Delay`, `Wait`, and now `TtyWrite` and `TtyRead`. We don't want to interpret a process on the blocked queue incorrectly, e.g. if a process if blocked writing to a terminal, we don't want to decrement its clock ticks to something negative! Because of this, we keep track of why process are blocked in the form of a variable `blocked_code` within the PCB.

- Processes that just called `Tty_Transmit` and are waiting for the trap are blocked with blocked_code `BLOCKED_TTY_TRANSMIT`
- Processes that want to call `Tty_Transmit` but it's currently being called are blocked with blocked_code `BLOCKED_TTY_TRANSMIT_LINE`



### TtyReceive

We initialize buffers of size `TERMINAL_MAX_LINE` in region0 for each terminal, these will store information. The buffers are `term0_buffer`, `term1_buffer`, `term2_buffer`, and `term3_buffer`

**KernelTtyReceiveTrap Pseudocode**:

```
TrapTTYReceiveHandler(void *ctx):
	error check user context
	
	call ttyreceive into a temporary buffer
	check the return code of ttyreceive to get size of line received
	
	if there's space in the buffer to add this information:
		move from temporary buffer into our dedicated buffer
		free the temporary buffer
	
	if there's no space
		give an error message that we've run out of space
```



**KernelTtyRead Pseudocode:**

```
KernelTtyRead(int tty_id, void *buf, int len):
	error checking
	
	if length of buffer is greater than len
		provide caller with len amount of buffer for the right terminal
	else if length of buffer is less than len
		provide caller with all of the buffer for the right terminal
```



**KernelTtyWrite Pseudocode:**

```
KernelTtyWrite(int tty_id, void *buf, int len):
	error checking
	if some other process is blocked doing ttytransmit on the same terminal:
		block the current process with blocked code blocked_tty_transmit_line
		
    copy buf into region0 space
    get the actual length of buf
    if the actual length is less than len:
    	give an error
   	
   	if len > TERMINAL_MAX_LINE
   		while there's still stuff to send:
   			call TtyTransmit
   			truncate the line
   			block process
   	else
   		call TtyTransmit
   		block process
   	
   	loop through blocked list, unblock the first process that was blocked waiting to do ttytransmit on the same terminal
   	
   	return len
```



```
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
            
            // swap process
            SwapProcess(NULL, uctxt);
        }
        
        // 
        ttyWriteTrackers[tty_id] = TERMINAL_CLOSED;
        int to_write = bytes_left > TERMINAL_MAX_LINE ? TERMINAL_MAX_LINE : bytes_left;
        char *ttyBuffer[to_write];
        memcpy(ttyBuffer, buf + bytes_written, to_write);
        TtyTransmit(tty_id, ttyBuffer, to_write);
        bytes_left -= to_write;
        bytes_written += to_write;
    }

	
    queue_pop(ttyQueue);
    if (ttyQueue->size > 0) {
        pcb_t *nextWriter = queue_peek(ttyQueue);
        nextWriter->blocked_code = NOT_BLOCKED;
        queue_add(ready_q, nextWriter, nextWriter->pid);
    }

    return bytes_written;
}
```



### Making Sure TtyTransmit or TtyReceive Isn't Called Simultaneously

When TtyWrite or TtyRead is called, the first thing we check is the blocked queue to see if any other process is already blocked because of TtyTransmit or TtyReceive. If it is and the caller's call is going to cause an issue, we block it.

At the end of our `KernelTtyWrite`, when we definitely finished transmitting, we loop through the blocked queue and move any process waiting to write. We do the exact same with `KernelTtyRead` for those processes wanting to TtyRead.

There is still a TOCTOU vulnerability though that could crash the OS. Once we code locks, and cvars, we need to require a lock for a kernel process to call TtyTransmit, and a separate lock for the kernel process to call TtyReceive.

## Todo:

- all traps and exceptions
- `TtyRead` and `TtyWrite` and `TtyTransmit`
  - for this, we need to set up our pipes, this should b the one done asap



## Questions

How do we access the stuff inside each given terminal?



