
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    TracePrintf(1,"fork.c: Calling fork()...\n");
    int rc = Fork();
    int pid = GetPid();
    int ppid = 0;
    if (rc == -1) {
        TracePrintf(1, "\n!!fork.c:Fork Syscall Failed!!\n");
    } else if (rc == 0) {
        TracePrintf(1, "fork.c: hello from child! PID -> %d\tPPID -> %d\n", pid, ppid);
    } else {
        TracePrintf(1, "fork.c: hello from parent!PID -> %d\tPPID -> %d\tCHILD_PID -> %d\n", pid, ppid, rc);
    }
    return 0;
}
