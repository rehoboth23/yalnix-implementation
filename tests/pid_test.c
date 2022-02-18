/*
 *  chkpnt3_test.c
 *  
 *  tests brk, getpid, and delay
 */
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(3,"Testing pid for checkpoint 3! \n");

    TracePrintf(3,"Calling get pid...\n");
    int pid = KernelGetPid();
    TracePrintf(3,"pid is: %d",pid);

    return 0;
}
