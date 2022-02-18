/*
 *  chkpnt3_test.c
 *  
 *  tests brk, getpid, and delay
 */
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(3,"Testing for brk checkpoint 3! \n");

    TracePrintf(3,"Calling brk with null!\n");
    Brk(NULL);

    TracePrintf(3,"Calling brk with a valid address\n");
    Brk(0xff002b);

    return 0;
}
