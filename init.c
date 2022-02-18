
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    while (1) {
        TracePrintf(1, "Nala Init Process - PID : %d\n", pid);
        Pause();
    }
    return 0;
}
