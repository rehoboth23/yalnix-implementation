/*
 *  chkpnt3_test.c
 *  
 *  tests brk, getpid, and delay
 */
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(1,"pid_test: Testing pid for checkpoint 3! \n");

    TracePrintf(1,"pid_test: Calling getpid()...\n");
    int pid = GetPid();
    TracePrintf(1,"pid_test: pid is: %d\n",pid);

    return 0;
}
