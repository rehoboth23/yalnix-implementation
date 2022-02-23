# Checkpoint 4 Writeup



## Data Structure Changes

We're no longer using a bit_vector to keep track of free frames but a linked list! The functions related to it are in `list.c` and `list.h`, instead of a global `bit_vector` being defined in `kernel.c`, a `pfn_list` is declared instead





**How did we implement round robin?**



### Testing

`init.c` tests Delay

`brk.c` tests Brk

`progs/fork.c` tests Fork

- tracing for this is in  `test_traces/FORK_TRACE`
- *lazy checking*: if you do `cat test_traces/FORK_TRACE | grep fork` you'll see that both the parent and child successfully print the correct pid and ppid. (refer to `progs/fork.c`  to see the TracePrintf command)
- *proper checking*: looking at `test_traces/FORK_TRACE`

`pid_test` tests GetPid

`exec1.c` and `exec2.c` tests Exec