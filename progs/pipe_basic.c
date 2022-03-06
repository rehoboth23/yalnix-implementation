
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"


int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "pipe_basic.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    char* hello = "Hello World!\n";

    TracePrintf(1,"Initializing pipe...\n");

    int *pipe = malloc(sizeof(int*));

    PipeInit(pipe);
    
    TracePrintf(1,"pipe_basic.c: Writing \"Hello World!\" to pipe...\n");

    PipeWrite(*pipe,hello,strlen(hello));

    char* read_result = malloc(32);

    TracePrintf(1,"pipe_basic.c:Reading 10 from pipe into a buffer...\n");

    PipeRead(*pipe,read_result,5);

    TracePrintf(1,"pipe_basic.c:Printing read_result: %s\n",read_result);
    
}