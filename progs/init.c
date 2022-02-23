
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    while (1) {
        TracePrintf(1, "init.c: PID -> %d\tPPID -> %d\n", pid, ppid);
        Delay(1);
        TracePrintf(1,"init.c: delay 2: PID -> %d\tPPID -> %d\n", pid, ppid);
        Delay(2);
        TracePrintf(1,"init.c: delay 3: PID -> %d\tPPID -> %d\n", pid, ppid);
        Pause();
    }
    return 0;
}
