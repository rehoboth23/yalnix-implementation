/*
 *  chkpnt3_test.c
 *  
 *  tests brk, getpid, and delay
 */
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(3,"Testing for checkpoint 3!\n");

    TracePrintf(3,"Calling get pid...\n");
    int pid = GetPid();
    TracePrintf(3,"pid is: %d",pid);

    TracePrintf(3,"Calling brk with null!\n");
    Brk(NULL);

    TracePrintf(3,"Calling brk with a valid address\n");
    Brk(0xff002b);

    TracePrintf(3,"Calling delay for 3 clock ticks\n");
    Delay(3);


    return 0;
}
