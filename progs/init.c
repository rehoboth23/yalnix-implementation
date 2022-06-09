
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    while (1) {
        TracePrintf(1, "init.c: PID -> %d\n", pid);
        Delay(1);
        TracePrintf(1,"init.c: delay 2: PID -> %d\n", pid);
        Delay(2);
        TracePrintf(1,"init.c: delay 3: PID -> %d\n", pid);
        Pause();
    }
    return 0;
}
