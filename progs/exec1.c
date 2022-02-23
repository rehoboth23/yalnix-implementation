
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "exec1.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    // makes log more readable
    Pause();
    
    Exec("progs/exec2", (char **) argv);
    return 0;
}
