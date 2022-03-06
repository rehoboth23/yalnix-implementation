
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int pid = GetPid();
    int ppid = 0;
    TracePrintf(1, "ipc_basic.c: PID -> %d\tPPID -> %d\n", pid, ppid);

    char* script = "According to all known laws of aviation, there is no way a bee should be able to fly. Its wings are too small to get its fat little body off the ground. The bee, of course, flies anyway because bees don\'t care what humans think is impossible.\
(Barry is picking out a shirt)\
\
Barry:\
Yellow, black. Yellow, black. Yellow, black. Yellow, black. Ooh, black and yellow! Let\'s shake it up a little.\
\
Janet:\
Barry! Breakfast is ready!\
\
Barry:\
Coming! Hang on a second.\
\
(Barry uses his antenna like a phone)\
\
Barry:\
Hello\
\
(Through phone)\
\
Adam:\
Barry?\
\
Barry:\
Adam?\
\
Adam:\
Can you believe this is happening?\
\
Barry:\
I can\'t. I\'ll pick you up.\
\
(Barry flies down the stairs)\
\
Martin:\
Looking sharp.\
\
Janet:\
Use the stairs. Your father paid good money for those.\
\
Barry:\
Sorry. I\'m excited.\
\
Martin:\
Here\'s the graduate. We\'re very proud of you, son. A perfect report card, all B\'s\
\
Janet:\
Very proud.\
\
(Rubs Barry\'s hair)\
\
Barry:\
Ma! I got a thing going here.\
\
Janet:\
You got lint on your fuzz.\
\
Barry:\
Ow! That\'s me!\
\
Janet:\
Wave to us! We\'ll be in row 118,000. Bye!\
\
(Barry flies out the door)\
\
Janet:\
Barry, I told you, stop flying in the house! ";

    char* receive = malloc(256);


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

        // try reading way too much
        TracePrintf(1,"stressful_pipes.c: child pipereading way too much...\n");
        rc = PipeRead(*pipe1,receive,1024);
        TracePrintf(1,"stressful_pipes.c: child got return code %d\n",rc);

        // try reading allowed amount
        TracePrintf(1,"stressful_pipes.c: child pipereading a legal amount\n");
        rc = PipeRead(*pipe1,receive,32);
        TracePrintf(1,"stressful_pipes.c: child read %s\n",receive);
        

        
    }
    else {
        int pid = GetPid();

        // trying to write way too much
        TracePrintf(1,"stressful_pipes.c: parent (pid %d) pipewriting way too much...\n");
        int rc = PipeWrite(*pipe1,script,strlen(script));
        TracePrintf(1,"stressful_pipes.c: parent got return code %d\n",rc);
        
        // trying to write max amount
        rc = PipeWrite(*pipe1,script,256);
        TracePrintf(1,"stressful_pipes.c: parent (pid %d) wrote to pipe1\n",pid);
        
        Delay(5);

        // char* to_write = "test Test tEst teSt tesT\n";
        // PipeWrite(*pipe1,to_write,strlen(to_write));        

    }
}