# Checkpoint 5 Writeup

**Did we make the checkpoint?** Yes! Wahoo again!



## Implementation

### TtyWrite

We have a `TtyQueue` for each terminal, so when a process wants to TtyWrite, we put it in that queue. Other processes that want to write to the same terminal are added to the same TtyQueue. A process can only call TtyTransmit if it is at the front of the queue.

**Avoiding simultaneous TtyTransmits to the same terminal**

- We created an array for TtyTransmit,` ttyWriteTrackers`, which is a global int array (an int for each terminal). It tells us whether or not a given terminal is free for TtyTransmit. In TrapHandlers.c after every TtyTransmit, we mark the terminal as free. We mark them as taken when we call TtyWrite and are able to write.
- Also, if a process is writing more than `TERMINAL_MAX_LINE`, we don't allow other processes to write to the same terminal until that first process is done. We do this by doing a neat trick where we swap process 



### Blocked Codes

There are lots of ways for a process to get blocked, `Delay`, `Wait`, and now `TtyWrite`. We don't want to interpret a process on the blocked queue incorrectly, e.g. if a process if blocked writing to a terminal, we don't want to decrement its clock ticks to something negative! Because of this, we keep track of why process are blocked in the form of a variable `blocked_code` within the PCB. And we have enums representing what each blocked_code represents in `include.h`

- Processes that just called `Tty_Transmit` and are waiting for the trap are blocked with blocked_code `BLOCKED_TTY_TRANSMIT`



### TtyReceive

We have a global array, `ttyReadbuffers`, an array of buffers for each terminal. We also have global queues for each terminal `ttyReadQueues[]`. We keep track of the size of the buffers by using `ttyReadTrackers[]`, which store the size of the buffer assigned for that terminal.



## Testing

### TtyWrite

`simul_ttywrite.c` forks and execs to `ttywrite.c`, one of the files writes "Good" and then "Goodbye World!", and the other file writes "Hello" and then "Hello World!", both to terminal 1. We fork to do this to make sure we're not calling `TtyTransmit` at the same time.

- if you run it, you'll see terminal 1 print 

- ```
  GoodGoodbye World!
  HelloHelloWorld!
  ```



`spam_ttywrite.c` tests the case where we write more than `TERMINAL_MAX_LINE` bytes. We fork and have a parent print a ton, then have the child simultaneously write a ton. We wanted to make sure that in the loop where the parent calls `TtyTransmit` multiple times, we don't interleave with the child, because the many`TtyTransmit` calls is the result of one `TtyWrite` call, so it should finish before we move onto another `TtyWrite`.

### TtyReceive and TtyRead

`ttyread_test.c` has 1 parent process and two children processes all interacting with the console at the same time. Each terminal writes a different message to the terminal console using `TtyPrintf`, and then each of them read using `TtyRead`, afterwards, they each read what they print to the terminal console.

When each process calls `TtyRead`, if you didn't write anything, they'll be blocked, waiting for something to be written to the terminal. Write something to the terminal and press enter. The first process with PID1 will print out what it read (which is what you wrote) and then its PID. You can do this two more times.

The order the process's called `TtyRead` in is the same order the processes read your terminal console input. This is because we're implementing a queue for when processes call `TtyRead` and are blocked.

*example test run:*

- run `./yalnix -x progs/ttyread_test`
- write "Hello" into terminal console
  - pid1 reads it, prints "Hello" in the terminal, and then prints "PID 1"
- write "What" into terminal console
  - pid2 reads it, prints "What" in the terminal, and then prints "PID 2"
- write "no" into terminal console
  - pid3 reads it, prints "no" in the terminal console, and then prints "PID 3"

### Other Trap Handlers

We test `TrapMemoryHandler` with the program `trap_mem.c`, which tries to touch a space within where the stack can grow, so it's a legal operation, and we enlarge the stack accordingly. Otherwise, we abort the process.



We test `TrapMathHandlers` and in turn `TrapIllegalHandler` (because they do the same thing) with the program `trap_math.c`, which tries to divide by 0. What we observe in the TRACE file (saved in `test_traces/MATH_TRACE`) is that the process with pid 0 is aborted and does not pop up again. 
