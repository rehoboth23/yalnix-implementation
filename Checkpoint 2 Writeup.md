# Checkpoint 2 Writeup

**Did we make the checkpoint:** I think so!

**Does it compile and run:** "I could've sworn it worked on my machine!"

- what we did to compile to run (where to put files etc):



## What we did

- created a file `boot.c` which contains `KernelStart` and `SetKernelBrk`. I think `SetKernelBrk` will end up in a separate source file, but it hasn't been created yet.

- For `KernelStart` (8.2.1 and 8.2.2)

  - we handle most switch cases, except for the esoteric onces, for them we only have pseudocode for now.
  - set up `bit_vector` to keep track of free frames
  - For the **two regions' page tables being set up**, I think this diagram does a good job showing what our understanding of yalnix's page table and how the hardware communicates with it.
    - ![image](pagetable_diagram.jpg)
    - given an address, we bitshift the address to the right by PAGESHIFT to turn the given address into page numbers, as described in 2.2.1
    - Then, we subtract this calculated virtual page number (or `vpn`) by the virtual page number of the 0th page, `vp0`. This gives us the index into the page table.
    - So for setting up Region 0's page table, we decided to turn each of the given addresses (e.g. `_kernel_data_start`, `_kernel_data_end`,`KERNEL_STACK_BASE`, etc) into virtual page numbers, then iterate through the indices of the page table, assigning pages if they fit in the ranges of each segment's `vpn` - `vp0`.
    - By doing this, we reserve the segments for `.data`, `.text`, the `heap` and the `stack`.

  - - 
  - set up Region 1 page table
    - found the first free frame above region 0's physical memory, and allocated 1 page to the idle's user stack
  - enabled virtual memory.

- Wrote `SetKernelBrk` in `boot.c`, it will likely be moved. (8.2.2)

  - use a global `kernel_brk` to keep track of where the brk is, also makes sure we don't mess things up when we enable virtual memory

- For the many Traps (8.2.3)

  - traceprint when there's a `TRAP_CLOCK`
  - traceprints the code of the syscall for `TRAP_KERNEL`s
  - and prints this trap is not yet handled in other handler functions.

- For settiing up `idlePCB` (8.2.4)

  - this happens within `boot.c` near the end of `KernelStart`
    - followed intructions
    - regarding "cook[ing] things so that when you return to user mode at the end of `KernelStart`, you return to this modified UserContext"



## Questions

In many of the `KernelStart` switches (e.g. the one that changes clock speed, or the default level), how do we actually implement that change for the rest of the OS? 

- i.e. right now, these values (clock speed, and the default level for hardware/kernel//user) are stored as globals, but nothing's being done with them.
- how do we use `helper_maybort(char *msg)`, I put in `helper_maybort("TracePrintf")` as a guess.

