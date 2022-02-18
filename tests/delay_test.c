/*
 *  chkpnt3_test.c
 *  
 *  tests brk, getpid, and delay
 */
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(3,"Testing for delay checkpoint 3! \n");

    TracePrintf(3,"Calling delay for 3 clock ticks\n");
    Delay(3);

    return 0;
}
