#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"
#include "hardware.h"

int main(int argc, char const *argv[])
{   
    Delay(30);
    int rc = Fork();
    if (rc == 0) {
        char buf[50];
        TtyPrintf(0, "Hello this is the child process\nPID %d\nSay Hi!\n", GetPid());
        TtyRead(0, buf, 120);
        TtyPrintf(0, "%sPID %d\n", buf, GetPid());
        return 0;
    }

    rc = Fork();
    if (rc == 0) {
        char buf[50];
        TtyPrintf(0, "Hello this is the child process\nPID %d\nSay Hi!\n", GetPid());
        TtyRead(0, buf, 120);
        TtyPrintf(0, "%sPID %d\n", buf, GetPid());
        return 0;
    }

    char buf[50];
    TtyPrintf(0, "Hello this is the parent process\nPID %d\nSay Hi!\n", GetPid());
    TtyRead(0, buf, 120);
    TtyPrintf(0, "%sPID %d\n", buf, GetPid());
    
    return 0;
}
