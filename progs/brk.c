
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {

    int pid = GetPid(); // get pid and print
    TracePrintf(1,"brk.c: PID -> %d\n",pid);

    int malloc_size = 100000;

    while (1) { // keep mallocing!!!
        TracePrintf(1,"brk.c: going to keep mallocing %d...\n",malloc_size);

        void* big = malloc(malloc_size);

        if (big == NULL) { // if malloc fails!
            TracePrintf(1,"brk.c: malloc returned NULL!\n");
        }
        else { // if malloc doesn't fail!
            TracePrintf(1,"brk.c: malloc successful, it's at %p\n",big);
        }
    }
}
