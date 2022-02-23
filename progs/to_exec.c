
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    int rc = 0;
    TracePrintf(0, "Nala exec program (%d) :\n\tPID -> %d\n\tPPID -> %d\n", argc, pid, ppid);
    return 0;
}
