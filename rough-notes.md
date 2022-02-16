# Yalnix Notes + TodoList



## Checkpoint 1: Pseudocode

### Requirements

- get used to build environment

- figure out how to move files back and forth between machine and environment
- examined **yalnix_framework/include**
- sketched all the kernel data structures
- pseudocoded all the traps, syscall, and major functions *<-- a good play to start imo*
  - it won't be final, but we need to go through the flow of all our code

The above, sketches and pseudocode, should be done in our real source-files as comments, and potentially with data structures, function stubs, and prototypes.

### Recap

- Missing `KernelStart`
- for virtual addresses we have options: pointers, page numbers, indices into a regions page table **DECIDE ON A WAY TO REFER TO VIRTUAL ADDRESSES**
  - lets do **indices**
- no need to support bi-directional pipes
- PCB: missing brk, space allocated for user stack (bruh what does that mean), a range of addresses? Should we do a start and end? => **WE MUST EDIT PCB TO ADD THESE**
- change syscall handler names, make it different from lib wrapper. **IMUST CHANGE SYSCALL HANDLER NAMES**
- PipeWrite: what if someone was blocked while waiting to read from this pipe
  - we need to signal them :D 
  - **UPDATE PIPEWRITE PSEUDOCODE**

## Checkpoint 2: Idle

### Requirements

- Write `void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt)`

  - where `cmd_args` is a vector of strings (like argv), each pointer to each argument from the boot command line, this is terminated by a NULL pointer
    - so we can iterate through it until NULL
  - `pmem_size` is the size of physical memory of the machine, this is given in bytes
  - `uctxt` is a pointer to our initial UserContext structure

- Set up virtual memory upon boot

  - have a way to keep track of free frames, hint: bit vector is the most concise, the slickest would be a linked list within the free frames themselves -> zero space overhead -> but tricky. He recommends starting with bit vector.
  - set up "initial Region 0" page table,the build process will tell us three things (refer to `yalnix.h`):
    - `void *_kernel_data_start`, the lowest address in our kernel data region
    - `void *_kernel_data_end`, the lowest address not used by kernel's instructions and global data, AT BOOT TIME
    - `void *_kernel_orig_brk`, the address the kernel library believes is brk at boot time
  - set up region 1 page table for idle, this one should have **one valid page**, for idle's user stack
  - Write `int SetKernelBrk(void * addr)`
    - `addr` is similar to what's used in calls to `brk`: it's the lowest location not used in our kernel
    - we need a flag in our kernel that checks if we have enable virtual memory, before it's enabled, ``SetKernelBrk` only needs to check if and by how kernel `brk` is being raised beyond `_kernel_orig_brk`, after VM is enabled, then our `SetKernelBrk` acts like the standard `Brk`. i.e. This function has two behaviors depending on whether or not virtual memory is enabled
      - the flag is provided by hardware, check 2.2.6, we call `WriteRegister(REG_VM_ENABLE, 1)` to set it,and I assume there's a ReadRegister function for checking.
    - when VM is enabled, if we already raised our kernel's brk since we've built the Region 0 page table, we need to adjust the page table before turning the VM on -- otherwise shit could get ugly

- Traps

  - set up interrupt vector, have it handle these handler functions to begin with
    - `TRAP_CLOCK`
    - `TRAP_KERNEL`
    - a "this trap is not yet handled" message, if we leave 0's at our unwritten trap and something occurs, things could get ugly.

- Idle

  - create an `idlePCB`, this is our kernel, as its first process, keep track of:

    - region 1 page table
    - kernel stack frames
    - UserContext (`uctxt` from `KernelStart`)
    - pid (from `helper_new_pid()`)

  - Write an idle function in the kernel text

    - ```
      void
      DoIdle(void) {
      
      	while(1) {
      	TracePrintf(1,"DoIdle\n");
      	Pause();
      	}
      }
      ```

    - In usercontext in `idlePCB`, set PC to point to our code, and set our SP to point towards the top of the stack set up

    - "cook things" so that when we return to user mode at the end of `KernelStart`, we return to this modified UserContext

    - ADD A TRACEPRINT "leaving KernelStart" at the end of `KernelStart`

## Checkpoint 3

### Requirements

Overview:

- kernel should load `init` into userland
- for an init with a `while(1)`loop that tracepeinrts and pauses, our kernel should bounce between dile and init on each clock trap.
- Kernel should handle `brk`, `getpid` and `delay`

Need to add an `init.c` in the USER part of the makefile

We should probably have a test file `test.c`

Ok! How do we get to this goal?



**8.3.1 A New Process** (most of it will be figuring out `KCCopy())`

- `KernelStart` creates a `initPCB`

  - `init`  is cloning into idle, make more sense (whatever that means)

    - what this means is 

    - ```c
      TracePrintf(0,"Cloning A into B\n");
      rc = KernelContextSwitch(KCCopy, B_PCB_p, NULL);
      TracePrintf(0,"We back, am I A or B?"); // both A and B prints this
      ```

    - 

  - new `initPCB` has:

    - empty (all invalid) region1 page table
    - new frames for its kernel stack frame
    - a `UserContext`, `uctxt` from `KernelStart`
    - a new pid with `helper_new_pid()`

- Write a `KCCopy()` to 

  - `KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *not_used)`
  - copies kernel context in `*kc_in` into the new pcb, and copy contents of the kernel stack into frames that have been allocated for the new process's kernel stack, then return `kc_in`
  - copy the current KernelContext into `initPCB`
  - copy the contents of the current kernel stack into the new kernel stack frames in `initPCB`
    - *how to do? temporarily map destination frame into some page, Sean likes the page right below kernel stack*

**8.3.2 Loading a Program **(will take quite a bit of work)

- Kernel will open `init` and set it up in region 1

- make a copy of `yalnix_framework/sample/template.c` and put it in our own directory, and edit it to match our kernel.

- look for places marked `==>>`, they indicate instructions to follow.

- figure 8.1 shows how it's meant to be done, in region 1's page table

  

**8.3.3 Specifying Init** (should b short and easy here)

- `cmd_args[0]` is what provides the `init` program for `KernelStart`, pass the whole `cmd_args` as arguments for the new process
- If no `cmd_args`, use `init` as a default program

**8.3.4 Changing Process**

- need to write `KCSwitch()` to:
  - `KernelContextSwitch(KCSwitch, (void*) &current_pcb, (void *) &next_pcb)` is how it's called
  - copy current KernelContext into the old PCB
  - change region 0 kernel stack mappings to thosse for the new PCB
  - return a pointer to the KernelContext in the new PCB



**8.3.5 TESTING!!!**

- test the three sys calls in a `test.c`, need to start a testing convention
- maybe a folder called "tests" and we put each of our tests in them, then call `./yalnix test1` or something like this?

### Implementation?

- in `kernel.c` have the queues of processes as globals there
- when we call sys calls, we get the current process by accessing the global queues, the only process in the running queue needs to be the prrocess that called the syscall
  - global running queues (defined in kernel.c are) `running_q`, `ready_q`, `blocked_q`, `defunct_q`.




**TODOs**

- write up queue.c, and then write up some testing code for it, then get the sys calls giong
  - test code will check for:
    - feeding invalid arguments for *every* queue helper function
    - adding a process to a queue it's already a part of
    - popping and removing from an empty queue
- 8.3.2, basically copy `template.c` and work through it, following the instructions in there so that we can load a program.
- 8.3.3 is tiny additional code at the start of `KernelStart`
- 8.3.4:
  - clock track handdler triggers a switch between 2 processes


### Questions 

- for writing `brk()`, once we know that we have enough virtual addresses free (contiguous virtual space), we allocate these virtual pages to the heap, however, what do we map the each pte to? in other words, how do we find the corresponding page frame number?
- 

## Notes

### Data Structures

**Process/PCB** 

- *page table* (structure provided for us in `hardware.h`)
  - each page table is an array of pagetable entries, here's how it's organized: 1st bit is valid bit, next 3 bits are protection bits (read, write, exec), next 4 bits are unused (so we can use that for our own chosen stuff), and the next 24 bits are the page and frame numbers. --> a total of 32 bits per entry
  - page table entry is defined as a data structure `pte_t` in `hardware.c`. It has the same mem layout as a hardware pagetable entry
  - if not enough space for bookkeeping, we can use a shadow page table.
- process ID
- PID of children (array of integers)
- number of children
- a `UserContext` struct, provided by `hardware.h`
- a `KernelContext` struct, provided by `hardware.h`
- location of user heap, user text segment, user data segment , user stack(as *virtual address* *indices* into the region's page table, so region 1)
- location of kernel heap, kernel text segment, kernel data segment, kernel stack (as *virtual address indices* into the region's page table, so region 0)
- address space allocated to this process (base + displacement, i.e. the first address and the size of the address space)
  - note: the address space var here may not be needed, but we'll keep for now.


**Queues**

- we can implement a queue using an array of pointers to processes, and have some helper functions defined in `queue.c` 
- We should make queues **dynamic arrays**, so its `size` is the num of elements in it

**Pipe**

- temporary kernel buffer, to allow for IPC, manual says "see the header files for the length of the pipe's internal buffer". cannot find it yet but this means our **pipe has a fixed length**, let's call it `p_max`.
- FIFO so it'll be a queue, but we can't use our queue data strcuture because it's specialized for handling processes. 
- **question:** do we have to support bi-directional pipes?
- supports the following IPC syscalls, they all return error if fail
  - `int PipeInit(int *pipe_idp)`
  - `int PipeRead(int pipe_id, void *buf, int len)`
  - `int PipeWrite(int pipe_id, void *buf, int len)`


We are provided a **User Context Structure**

- defined in `hardware.h`
- contains hardware state of the currently running user process
  - `int vector`, the type of interrupt/exception/trap, it's an index into interrupt vector table
  - `int code`, more info on interrupt/executable/trap, check manual
  - `void *addr`, only meaningful for `TRAP_MEMORY` exception as it will store the memory address whose reference caused the exception
  - `void *pc`, PC value at time of interrupt/exception/track
  - `void *sp`, stack pointer value at time of interrupt/exception/track
  - `void *ebp`, frame base pointer at the time of interrupt/exception/track
  - `u_long regs[8]`, contents of eight gen purpose CPU registers at time of interrupt/exception/trap

**Interrupt Vector Table** <-- just an array of pointers

- stored in memory as an array of pointers to functions, each function handles corresponding interrupt/exception/trap. each type of interrupt/exception/trap is defined in `hardware.h` as a symbolic constant, the number is used by the hardware to index into our interrupt vector table to get the appropriate handler function
- the functions we point to are handler functions for different interrupt/exception/traps
- when we call these functions we give it a single argument, the pointer to a `UserContext`, the functions always return void
- must have exactly `TRAP_VECTOR_SIZE` entries in it, even if the use for it is undefined in hardware.

We are provided **KCSFunc_t**, **Kernel Context Switch Function Type** 

- defined in `hardware.h`
- given as an argument when we call `int KernelContextSwitch`, cause it's a special kind of function that takes:
  - a `KernelContext` pointer and two void pointers as arguments



**Kernel Heap**

- we use kernelland malloc, which invokes `SetKernelBrk` when it must adjust kernel heap

**Kernel Stack**

- is a fixed maximum size: `KERNEL_STACK_MAXSIZE`
- at the same address in virtual memory for all processes
- always beings at virtual address `KERNEL_STACK_BASE`
- first byte beyond stack has address `KERNEL_STACK_LIMIT`, which is the top of Region0 of virtual memory
- There's a red zone below the stack, one or two pages of unmapped memory

We're given a pagetable struct in `hardware.h`

### Big Picture

![Big Picture](/home/kris/.config/Typora/typora-user-images/image-20220124214033213.png)

For the diagram above, we are writing the **kernel code** (light gray boxes), and we may after write some **application code** (dark gray) BUT we are provided samples.

**What each transition number corresponds to**:

1. hardware invokes `KernelStart()`to boot the system
2. returning of `KernelStart`, the machine then begins running in user mode at a specified UserContext
3. called when kernel library needs to **adjust the top of the kernel heap**, library calls `SetKernelBrk()`, which we will write.
4. Our kernel code will **call special machine instructions** linked with hardware.h to do things like change page table pointers
5. trap handler, whenever there's a **syscall, interrupt, or exception**
6. when our **trap handler returns** to application code
7. **Translation Look-Aside Buffer (TLB) Misses**: if user land or kernel land touches an address that isn't in TLB, we handle that miss according to what it believes are the current page tables.
8. **Calling `KernelContextSwitch()` **, kernel change to a different execution context
   1. this is tricky, all our registers and PC and stack pointers need to be changed while using those same registers. We also need to save the userContext of the running process into some data structure, it is restored by passing the UserContext pointer into the trap handler (once in trap handler, hardware will take it from there)
   2. Cause it's tricky, we're provided a way for our kernel code to invoke kernel context-switching functions, which we'll write, in a special context

Kernel code is linked to a provided **kernel library**, this includes routines like `malloc()` and others, for dynamic allocation from the kernel heap.

### Zooming in on each transition

**Transition 1 and 2 KernelStart()**

- Bootup, we need to write `KernelStart()` which the hardware calls
  - we must initialize:
    - Interrupt vector table, and write the address of it to **REG_VECTOR_BASE** register, three main handler functions tho
      - one for `TRAP_CLOCK`, traceprints when there's a clock trap
      - one for `TRAP_KERNEL` trap prints in hex, the code of the syscall
    - virtual memory
      - set up a way to free frames (use a **bit vector**, but it is hard to track, maybe start with something safer?)
    - set up initial Region 0 pagetable, the build process gives us three addresses to help us start
      - `void *_kernel_data_start`: lowest address in kernel data
      - `void *_kernel_data_end`: lowest address not in use by kernel's instructions and global data at boot time
      - `void *_kernel_orig_brk`:: address that kernel lib believes is its brk at boot time
      - check `yalnix.h`
    - set up Region 1 page table for idea, it should have one valid page, for the idle's user stack
    - write `int SetKernelBrk(void *addr)`
    - enable virtual memory, adjust page table before turning virtual memory on
  - We then run idle in user mode
  - have a traceprint "leaving KernelStart" at the end of `KernelStart`
- then we return to some UserContext in user mode

**Transition 3 SetKernelBrk()**

- when `malloc` is called, we need to write `SetKernelBrk()` to adjust top of kernel heap

**Transition 4: special machine instructions**

- we need to call special machine instructions to do many thins:
  - change page table pointersTransition 8 and 9
  - we are provided 
  - I think we also need to write `void Pause(void)` and `void Halt(void)` , details are in 2.6

**Transition 5 and 6: syscalls, traps, exceptions** 

- we need to write the trap handler that switches to kernel mode, handle the syscall, interrupts, exceptions, and return hardware to user mode correctly
  - go to txtbook chapter 2 and 3
  - for the pseudocode of the syscalls below, shall we start at the point where we're already in Kernel mode with our context set up?
  - syscalls, `yalnix.h` defines constant syscall number for each system (the syscall num is in the `UserContext` struct that's passed by the hardware for the trap), yuser.h defines the function prototypes, but the manual has the most info.
    - basic process coordination 3.1.1
      - `int Fork(void)`
      - `int Exec(char *filename, char **argvec)`
      - `void Exit(int status)`
      - `int Wait(int *status_ptr)`
      - `int GetPid(void)`
      - `int Brk(void *addr)`
    - I/O syscalls 3.1.2
      - `int Delay(int clock_ticks)`
      - `int TtyRead(int tty_id, void *buf, int len)`
    - Inter-Process Communication (IPC) 3.1.3
      - `int PipeInit(int *pipe_idp)`
      - `int PipeRead(int pipe_id, void *buf, int len)`
      - `int PipeWrite(int pipe_id, void *buf, int len)`
    - Synchronization syscalls 3.1.4, these operate on *processes*
      - `int LockInit(int *lock_idp)`
      - `int Acquire(int lock_id)`
      - `int Release(int lock_id)`
      - `int CvarInit(int *cvar_idp)`
      - `int CvarSignal(int cvar_idp)`
      - `int CvarBroadcast(int cvar_idp)`
      - `int CvarWait(int cvar_idp, int lock_id)`
      - `int Reclaim(int id)`
  - Our OS must be able to handle the following traps. We do this by creating an **interrupt vector table**, and initializing each entry in the table with a pointer to a handler function in the Kernel. The address of this vector table must be written into the **REG_VECTOR_BASE** register by the kernel *at boot time*.
    - 3.2 in the textbook
    - `TRAP_KERNEL`, check `code` of `UserContext` passed by reference to the trap handler function, and execute the syscall it refers to
      - args to syscall is found in registers
      - return to user process in `regs[0]` of `UserContext`
      - this occurs when a user process requests a syscall for some function provided by the kernel, THIS is how the user calls the kernel
    - `TRAP_CLOCK`, context switch with another process on ready queue, if it exists, if there are none, "dispatch idle" *<-- what does that mean*
    - `TRAP_ILLEGAL`, abort current user process, but continue other processes
      - when we abort, we should `TracePrintf` a message at level 0, and give the PID, and some explanation. Also make the exit status reported to the parent process of the aborted process when the parent calls `Wait` be `ERROR`
    - `TRAP_MEMORY`, determine if the exception was an implicit request to enlarge memory allocated on the current process's stack
      - if so, enlarge stack to cover the address referenced that caused the exception, then return from exception
      - otherwise, abort the process, but continue other processes (maybe we can refer to whatever handles TRAP_ILLEGAL here)
        - when we abort, we should `TracePrintf` a message at level 0, and give the PID, and some explanation. Also make the exit status reported to the parent process of the aborted process when the parent calls `Wait` be `ERROR`
    - `TRAP_MATH`, abort current user process, but continue other processes
      - when we abort, we should `TracePrintf` a message at level 0, and give the PID, and some explanation. Also make the exit status reported to the parent process of the abrocess_idorted process when the parent calls `Wait` be `ERROR`
    - `TRAP_TTY_RECEIVE`, indicates that a new line of input is available from terminal indicated by the `code` of `UserContext` passed to interrupt handler function. 
      - We should call `TtyReceive` to read the input, and if necessary "buffer the input line for a subsequent `TtyRead` syscall by some user process" *<-- not sure what this means*
    - `TRAP_TTY_TRANSMIT`, indicates that the previous `TtyTransmit` hardware op has been completed, the terminal is specified in the `code` of `UserContext` passed to our interrupt handler function
      - we should complete the blocked process that started the terminal output "from a `TtyWrite` syscall, also start the next terminal output on this terminal, if any"
    - `TRAP_DISK`, ignore, unless we add extra functionality for the disk

- e.g. to go through what happens when userland code makes a syscall (fork)
  - user code calls `fork` --> /etc/yuserlib/calls.c then calls `YSYSCALL(YALNIX_FORK,0,0,0,0)`--> yalnix.h finds that YALNIX_FORK is 0x1 --> trap instruction is generated by call.c --> check interrupt vector table --> call corresponding function handler for `TRAP_KERNEL` --> check which syscall it is at `code` of `UserContext` --> call function handler for specific syscall
  - our return code should be put in `regs[0]` for the function wrapper
  - afterwards, hardware restores `UserContext` for us
  - notice that a lot of the work is done by context switching helpers, so `fork`is a bad example

**Transition 7 TLB misses**

- TLB (basically cache) misses

**Transition 8 and 9 context switching**

- Kernel Context Switching
  - so changing all registers, pc and stack pointer
  - We should use the provided C typedef function `KernelContext *KSCFunc_t(KernelContext *,void *, void *)` , the two void* pointers point to the current PCB and the PCB we'll change to
    - we need to **check hardware.h** for details on this one
  - Context Switching Functions
    - `int KernelContextSwitch(KCSFunc_t *, void *, void *)`
      - takes in KSCFunc_t which takes in a `KernelContext` pointer, and two void pointers, and returns a `KernelContext`
      - our kernel code will use this function by passing it a function of type KSCFunc_t and two void pointers
      - This function does two distinct tasks *we're recommended to use two different functions (one for each task*
        - switching (in kernel-level) from one process to another
        - cloning a new (kernel-level) process in the first place (so we can switch to it)
      - returns 0 if successful, otherwise -1, where we should print an error and exit
      - this function does not move PCB from queues, and does not do any bookkeeping, it will not do any page table stuff either, all this does is it **helps us save the state of the process and later restore it**, our **kernel must take care of itself**
    - `KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p)`
      - takes in pointer of current kernel context, and two PCB pointers
      - this is one of the functions that substitute as `KSCFunc_t`
    - `KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *not_used)`
      - copies kernel context from kc_in into the new pcbb, copy the contents of the current kernel stack into frames allocated for the process's kernel stack
      - returns kc_in

### Coding and Running 

Kris: it currently *seems* like we're meant to create two source files, one **kernel source** and one **user source**, and a bunch of include files for each of the header files provided

Ultimately, Yalnix is a userland process running on a Linux on an Intel machine.

When compiling yalnix_framework: we use `yalnix_framework/sample/Makefile`, which currently contains the basis for the `Makefile` , we can ignore the magic of how this Makefile sets up compiling and linking with `gcc`, although it would be cool to understand it.

Running yalnix may generate Linux processes called `yalnixtty`, `yalnixnet`, and `yalnix`, we can kill it with `kill` if we must.

Three components to our system:

- kernel code
  - all kernel code files should `#include <ykernel.h>`
- user code
  - all user code file should `#include <yuser.h>`
- underlying hardware code

All of these components have access to `TracePrintf(int level, char *ft, args...)`, level is the integer tracing level, fmt is a format specification in the style of printf, args is the arguments for fmt. Both kernel and user code can use functions in `ylib.h`, don't use other lib functions.

- We can run our kernel with the tracing level set to some integer. Traceprints are sent to `stderr` as well as a file called `TRACE`. 
- **Use this properly and consistently while developing the project.** e.g. a low-level `TracePrintf` at the beginning and end of each function, (e.g. look at `ENTER` and `LEAVE` in hardware.h), this could be a good coding convention. 
- Trace levels 1 or higher will print out any of the principal transitions in the Figure above.

I'll just paste from section 5.4 of the manual for this one man:

=====================================================================================

**5.4 Running Yalnix**

Your kernel will be compiled and linked as an ordinary Linux executable program, and can thus be run as
a command from the Linux shell prompt. When you run your kernel, you can put a number of Linux-style
switches on the command line to control some aspects of execution. The file name for your executable
Yalnix kernel should be **yalnix**. You can then run your kernel as:

```./yalnix (yalnix options) (KernelStart options)```



**5.4.1 KernelStart Options**
If these are blank, then your kernel (from Checkpoint 3 onwards) will look for a executable called “init”.
Otherwise, the KernelStart options would be passed into KernelStart as **cmd_args**. E.g., if you typed

```./yalnix alt init a b c```

then your KernelStart would be called with cmd args as follows:

```
cmd_args[0] = "alt_init"
cmd_args[1] = "a"
cmd_args[2] = "b"
cmd_args[3] = "c"
cmd_args[4] = NULL
```

Your kernel would then look for **alt_init** to load as its initial process.



**5.4.2 Common Yalnix Options**
• -**x** : Use the X window system support for the simulated terminals attached to the Yalnix system. The
default is not to bother. (You will probably not require -x until you get to Checkpoint 5.)
• -**lk** level: Set the tracing level for this run of the kernel. The default tracing level is 1—which is
how we will test your code. You can specify any level of tracing.
• -**lh** level: Like -lk, but sets the trace level to be applied to internal TracePrintf calls made by
the hardware simulation. The default tracing level is 1.
• -**lu** level: Like -**lk** and -**lh**, but sets the tracing level applied to TracePrintf calls made by
user-level processes running on top of Yalnix. The defult tracing level is 1.
• -**W**:̇ This tells yalnix to stop and dump core if the hardware helper (see Chapter 6 below) encounters
any significant problems. (Without -W, the helper will merely traceprint about them; with -W, you
can then use gdb to examine the core and see what caused it.)



**5.4.3 Esoteric Yalnix Options**
• -**t** tracefile: Send traceprinnts to tracefile instead of TRACE.
• -**C** **NNN** : The default tick interval is allegedly1 400 ms. This option lets you change it to some other
number of millisecons (e.g., to speed it up).
• -**I**n file: feed terminal n with the data from file, as if it were typed there.
• -**O**n file: By default, the output from terminal n goes to a file named TTYLOG.n. This option lets you
change that to file.

=====================================================================================

## Testing

### Checkpoint 1 and 2

- test kernel's malloc and setkernelbrk
  - see what happens when malloc fails, does it fail? maybe do a program that mallocs until it fails
  - see what happens if we set kernelbrk before initializing memory
- test our memory regions
  - in a working process, lets try printing all our kernel memory segment address just to see that it lines up
  - maybe even print our bit vector and trace it through that

### Oh Shit Reminders:

- Kernel is **not interruptible!** don't use synchronization stuff like locks or cvars when *inside* the kernel.
- keep printing with `TracePrintf` on all levels, for debugging!!!!
  - put at the start and end of each function!!
- we are given a magical function that can force a context switch from a process to another, while inside the kernel, then later resume the execution of the blocked kernel-mode process
- flush the TLB at every context switch so we don't get incorrect hits
- if we change kernel brk before enabling virtual memory, we musts update the page table!!

## Questions

it says on chapter 4 that processes call `KernelContextSwitch`, where does it do that
