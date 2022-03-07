The pipe is implemented as a struct that contains the following information.

We store multiple pipes as a linked list, utilizing a `next` variable within a pipe_t struct

- void* of size PIPE_BUFFER_LEN, the buffer representing the actual pipe
  - initialized to 0
- int plen, the current number of bytes being used
- int id, identifying the pipe
- pipe* next
- int being_used, a flag variable to identify whether or not a process is currently interacting with the pipe
  - to prevent other processes from reading and writing while a pipe is being read/written to

Similar to termianls, we're also implementing a queue of processes waiting to write and processes waiting to read

Note that the head pipe, the pipe with id 0 is not used, if the user initializes a pipe for the first time, they're given the pipe with id 1.