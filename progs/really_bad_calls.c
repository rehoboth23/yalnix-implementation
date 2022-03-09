#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {


    int rc;

    // calling bad sys calls and printing the returncode.    

    TracePrintf(1,"really_bad_calls.c: calling wait with no child\n");
    rc = Wait(NULL);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling brk with NULL\n");
    rc = Brk(NULL);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling bad ttyread\n");
    rc = TtyRead(9,NULL,-1);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling bad ttywrite\n");
    rc = TtyWrite(5,"bad bad bad",6);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling piperead without pipe existing\n");
    rc = PipeRead(5,"hello?",50);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling release with nonexistent lock\n");
    rc = Release(69);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling CvarSignal with invalid id\n");
    rc = CvarSignal(420);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling CvarBroadcast with invalid id\n");
    rc = CvarBroadcast(1337);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling Reclaim with invalid id\n");
    rc = Reclaim(2000);
    TracePrintf(1,"really_bad_calls.c: got return code %d\n",rc);

    TracePrintf(1,"really_bad_calls.c: calling exec with invalid program\n");
    rc = Exec("invalid","suuuper invalid");
    // exec won't return, we kill the process



}