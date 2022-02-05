/*
 *  process.h
 *  
 *  holds data struct for a process, we're calling it a PCB.
 *  also holds the functions for manipulating the process
*/

typedef struct PCB {} pcb_t;

// NOTE: I'm not sure about this function, we definitely need to
// initialize a lot of stuff, but the details are quite hard to

int process_init(pcb_t **processBlock);