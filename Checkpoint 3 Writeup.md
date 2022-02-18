# Checkpoint 3 Writeup

**Did we make the checkpoint:** Not quite. We have the main functionality down, but some errors in our queue data structure is preventing us from testing `Brk()` and the other functions.

- we don't take in other test programs yet
- we successfully swap between an `init` and an `idle` process



We're currently using a workaround instead of our ready and running queues to swap between our two processes, we're using a flag variable called `status` within `initPCB`'s struct to keep track of which process is currently running. 



This is because we **still have bugs in our queue data struct** and its helper functions. So we couldn't implement things that other members already coded.



## What we did

- For KernelContextSwitches, we created the new files `contextswitch.c` and `contextswitch.h` which store the functions `KCCopy` and `KCSwitch`, its behavior is as specified in the manual
- For 8.3.2, we copied `template.c` to create a new file: `load.c`
- For 8.3.3, we added an if condition at the start of `KernelStart` that sets a variable `prog` to "init" if there's no input, otherwise it's set to whatever cmdline input there is. For now, we 
- For 8.3.4, KCSwitch is in `contextswitch.c` and works.
  - the `TrapClockHandler` will check our global ready queue `ready_q` to see if there are processes that we can switch to, if so, then it calls `KernelContextSwitch`, passing in `KCSwitch` and the two processes it's swapping between. <-- This is in `not-implemented-traphandlers.c`. 
  - so we added to `TrapClockHandler` and `KCSwitch` to allow this to work

- **We have testing ready but haven't reached the point where we can do it.** 



Wrote code for `Delay`, `Brk`, and `GetPid`, but some errors in the queue data struct doesn't allow us to test it. Right now, we're using an outdated `traphandlers.c` to run our code, but the code written for these three functions are in `not-implemented-trap-handlers.c`.



Implementing `Delay()` (in `not-implemented-traphandlers.c`)

- when `KernelDelay()` is called, `clock_ticks` variable in the target process's `pcb` is set to the number of clock ticks provided in the `KernelDelay()` function call
- with each clock interrupt, if any process' `clock_ticks` is above 0, it is decremented
  - Case 1: process being delayed is the only process that's ready
    - we don't move the target process to the blocked queue, all we can do is wait and decrement after each clock tick
  - Case 2: there are other processes to run when one process is delayed
    - if we haven't done so already, we move our target process to the blocked queue, and any ready process to the running queue
    - we also check each element of the blocked queue, decrementing the `clock_ticks` in any that have a positive number of `clock_tick`s.



Implementing `Brk()` (in `not-implemented-traphandlers.c`)

- first we do some error checking to see if the address provided is valid
  - if it's below our current brk, we won't allow it
  - if it makes our heap grow into our stack or beyond, we won't allow it
- otherwise, we loop through our region1 space between the old brk and the suggest brk twice
  - 1st loop: check that every virtual address between the old and new brk, if any of them are taken, return error as the brk has failed
  - 2nd loop: we only reach here if we get through the first loop, this time we allocate each page as we traverse from our old brk to our new brk, with each page, we find a corresponding physical frame to map our virtual addresses to.





## How to Compile and Run

Currently, the user directory is the same as the kernel directory, just so it's easily accessible by our IDEs.

So everything is in the same directory, `make` and `./yalnix -W` should do the trick.

## 

## Testing!

Although we didn't reach a stage where we took in testing functions, we wrote test programs so that once the functionality has been included, we can test the sys calls that we've written. These tests are in `tests`.



To test what we've implemented so far, `./yalnix -W` will show that we're successfully switching contexts between idle and init.





