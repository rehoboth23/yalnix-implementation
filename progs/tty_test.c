#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[])
{
    int rc = Fork();
    char buff[1024];
    if (rc == 0) {
        int pid = GetPid();
        TracePrintf(0, "%s\nPID %d\n", "Hello friend\nI am the child process\nCan you see this text?", pid);
        TtyPrintf(0, "%s\nPID %d\n", "Hello friend\nI am the child process\nCan you see this text?", pid);
        TtyRead(0, buff, 1024);
        TracePrintf(0, "Read From TTY\nSTART\n%s\npid %d\nEND\n", buff, pid);
        TtyPrintf(0, "Read From TTY\nSTART\n%s\npid %d\nEND\n", buff, pid);
        return 0;
    }
    int pid = GetPid();
    TracePrintf(0, "%s\nPID %d\n", "Hello friend\nI am the parent process\nCan you see this text?", pid);
    TtyPrintf(0, "%s\nPID %d\n", "Hello friend\nI am the parent process\nCan you see this text?", pid);
    TtyRead(0, buff, 1024);
    TracePrintf(0, "Read From TTY\nSTART\n%s\npid %d\nEND\n", buff, pid);
    TtyPrintf(0, "Read From TTY\nSTART\n%s\npid %d\nEND\n", buff, pid);
    return 0;
}
