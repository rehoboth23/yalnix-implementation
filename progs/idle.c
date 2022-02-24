
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    int rc = 0;
    while (rc != -1) {
        TracePrintf(0, "Nala idle program :\n\tPID -> %d\n\tPPID -> %d\n", pid, ppid);
        Pause();
    }
    return 0;
}
