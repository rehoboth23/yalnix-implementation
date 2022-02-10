/*
 *  traphandlers.c
 *  
 *  Defines function handlers for all traps/syscalls
 *  Trap handlers are referred to by the interrupt vector table.
 *  The syscall handlers are called by TrapKernelHandler
 * 
*/

#include <ykernel.h>
#include "pipe.h"
//#include "interrupt.h"

typedef void (*handler_func_t)(void *ctx); // I don't know yet the best way to phrase the generic function type of a handler
handler_func_t **InterruptVectorTable;

void TrapKernelHandler(void *ctx);


void TrapClockHandler(void *ctx);


void TrapIllegalHandler(void *ctx);

void TrapMemoryHandler(void *ctx);

void TrapMathHandler(void *ctx);


void TrapTTYReceiveHandler(void *ctx);

void TrapTTYTransmitHandler(void *ctx);

void TrapDiskHandler(void *ctx);