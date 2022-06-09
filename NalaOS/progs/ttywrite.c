
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"


int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "ttywrite.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    char* hello = "Hello World!\n";
    
    // Write Hello
    TracePrintf(1,"Writing Hello\n");
    TtyWrite(1,hello,5);

    // Write all of it
    TracePrintf(1,"Writing Hello World!\n");
    TtyWrite(1,hello,13);

    // // Write even more
    // TracePrintf(1,"Writing way too much\n");
    // TtyWrite(1,hello,1024);
}
