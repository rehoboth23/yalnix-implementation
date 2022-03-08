## Implmentation

### Pipes

The pipe is implemented as a struct, many pipes would then be implemented as a linked list of structs. 

We store multiple pipes as a linked list, utilizing a `next` variable within a pipe_t struct. Each struct contains the following

- `void* buf`of size PIPE_BUFFER_LEN, the buffer representing the actual pipe
  - initialized to 0
- `int plen`, the current number of bytes being used
- `int id`, identifying the pipe
- `pipe* next`
- `int being_used`, a flag variable to identify whether or not a process is currently interacting with the pipe
  - to prevent other processes from reading and writing while a pipe is being read/written to
- `queue_t *queue`

Similar to terminals, we're also implementing a queue of processes waiting to write and processes waiting to read

Note that the head pipe, the pipe with id 0 is not used, if the user initializes a pipe for the first time, they're given the pipe with id 1.



## Testing

### Pipes

`pipe_basic.c` 

- not interprocess communication yet
- tests that a single process can write to a pipe and then read an allowed number of bytes from it.
- test tracing can be found at `test_traces/PIPE_BASIC_TRACE`
  - doing `cat test_traces/PIPE_BASIC_TRACE | grep pipe_basic.c` will show that the process successfully wrote to the pipe, then read the right information from it

`ipc_basic.c`

- like in the name, this tests Inter-Process Communication
- two processes, one writes and one reads
- the idea is the two processes can only get the information from the pipe, because they don't share region1.
- child process will read more than what the parent wrote, so the child process will read the entire pipe
- test tracing can be found at `test_traces/IPC_TRACE`
  - doing `cat test_traces/IPC_TRACE | grep ipc_basic` will show that the parent process wrote to the pipe, the child process read successfully from it, then  the same thing is repeated again, except this time the child tried to read before the parent wrote something, so the child is blocked until the parent writes more.

`stressful_pipes.c`

- makes sure our pipes are failing gracefully when something illegal is done
- Two processes, child and parent
  - we start with the parent trying to pipewrite way too much and print the return code of that attempt
  - then we have the parent write an acceptable amount
  - we have the child try to piperead way too much and print the return code of that attempt
  - then we have the child read a chunk of what the parent wrote
- test tracing can be found at `test_traces/PIPES_STRESS_TRACE`
  - doing `cat test_traces/PIPES_STRESS_TRACE | grep stressful ` will show that bad calls to the pipe syscalls give a return code of -1, and that legal writes and reads are done successfully, because the child is reading a chunk of the bytes written by the parent, it can't print it as a string through `grep`, but you can see that the piperead worked by looking at the tracefile.