
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"


int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "simul_ttywrite.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    char* bye = "Goodbye World!\n";
    
    int rc = Fork();

    if (rc == 0) {
        char* args[2];
        args[1] = "progs/ttywrite";
        Exec("progs/ttywrite",args);
    }

    // Write Hello
    TracePrintf(1,"Writing Good\n");
    TtyWrite(1,bye,4);

    // Write all of it
    TracePrintf(1,"Writing Goodbye World!\n");
    TtyWrite(1,bye,15);

}