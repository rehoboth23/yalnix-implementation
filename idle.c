
#include "yalnix.h"
#include "ylib.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int a = 56;
    char str[256];
    while (1) {
        TracePrintf(1, "Nala Idle Process - PID : %d\n", pid);
        Pause();
    }
    return 0;
}
