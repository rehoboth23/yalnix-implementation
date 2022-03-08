
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"


int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "ipc_basic.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    char* hello = "Hello Other Process!\n";

    char* receive = malloc(32);


    // lets set up 2 pipes
    int *pipe1 = malloc(8);
    PipeInit(pipe1);
    TracePrintf(1,"ipc_basic.c: got first pipe id %d\n",*pipe1);

    int *pipe2 = malloc(8);
    PipeInit(pipe2);
    TracePrintf(1,"ipc_basic.c: got second pipe id %d\n",*pipe2);


    int rc = Fork();

    if (rc == 0) {
        // wait for parent to write
        Delay(2);
        // trying to receive 9 when there are only 5.
        PipeRead(*pipe1,receive,9);
        int pid = GetPid();
        TracePrintf(1,"ipc_basic.c child (pid %d) read \"%s\" from pipe1\n",pid,receive);

        TracePrintf(1,"ipc_basic.c: child (pid %d) going to try to read more...\n",pid);
        PipeRead(*pipe1,receive,32);

        TracePrintf(1,"ipc_basic.c: child (pid %d) read %s from pipe1\n",pid,receive);
    }
    else {
        // write "Hello"
        PipeWrite(*pipe1,hello,5);
        int pid = GetPid();

        TracePrintf(1,"ipc_basic.c: parent (pid %d) wrote to pipe1\n",pid);
        
        Delay(5);

        TracePrintf(1,"ipc_basic.c: parent (pid %d) wrote more to pipe1\n",pid);
        char* to_write = "test Test tEst teSt tesT\n";
        PipeWrite(*pipe1,to_write,strlen(to_write));        

    }
}