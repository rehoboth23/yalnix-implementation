
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    
    int fc = Fork();
    if (fc == 0) {
        char *args[2];
        args[1] = "progs/to_exec";
        args[2] = "Child 1";
        Exec("progs/to_exec", args);
    }
    fc = Fork();
    if (fc == 0) {
        char *args[2];
        args[1] = "progs/to_exec";
        args[2] = "Child 2";
        Exec("progs/to_exec", args);
    }

    int rc = 0;
    while (Wait(&rc) != -1) {
        TracePrintf(1,  "wait_exit.c: PID -> %d\tmessage: child has exited with code (%d)\n", pid, rc);
    }

    TracePrintf(1, "wait_exit.c: Exiting out of wait_exit (pid %d)\n", pid);
    return 0;
}
