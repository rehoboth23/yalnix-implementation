
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    TracePrintf(1, "to_exec.c: PID -> %d\n", pid);
    if (pid == 2) {
        return 2;
    }
    else {
        return 0;
    }
}
