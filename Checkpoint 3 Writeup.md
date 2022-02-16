# Checkpoint 3 Writeup

**Did we make the checkpoint:**





## What we did

- For KernelContextSwitches, we created the new files `contextswitch.c` and `contextswitch.h` which store the functions `KCCopy` and `KCSwitch`, its behavior is as specified in the manual
- For 8.3.2, we copied `template.c` to create a new file: **<u>*PUT FILE NAME HERE*</u>**
- For 8.3.3, we added an if condition at the start of `KernelStart`
- For 8.3.4, the `TrapClockHandler` will check our global ready queue `ready_q` to see if there are processes that we can switch to, if so, then it calls `KernelContextSwitch`, passing in `KCSwitch` and the two processes it's swapping between.
  - so we added to `TrapClockHandler` and `KCSwitch` to allow this to work
- We also did testing! Will be specified below in the Testing! section.





Implementing `Delay()`

- when `KernelDelay()` is called, `clock_ticks` variable in the target process's `pcb` is set to the number of clock ticks provided in the `KernelDelay()` function call, with each clock interrupt, the process's clock ticks is decremented
  - Case 1: process being delayed is the only process that's ready
    - we don't move the target process to the blocked queue, all we can do is wait
  - Case 2: there are other processes to run when one process is delayed
    - so then we move our target process to the blocked queue, and check the blocked queue at every clock tick

## How to Compile and Run



## Testing!

Tested `Brk()`, `GetPid()`, `Delay`.





