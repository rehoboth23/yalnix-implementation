
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"



int main() {
    
    int rc = Fork();
    char buff[1024];
    if (rc == 0) {

        while (1) {
            char *src = "Hello This is the child process\0";
            int cpy_len = strlen(src);
            int len = cpy_len * 60 + 1;
            int i;
            char buf[len];
            for (i = 0; i < len; i += cpy_len) {
                memcpy(buf + i, src, cpy_len);
            }
            buf[i] = '\0';
            TtyPrintf(0, "%s", buf);
            TtyPrintf(0, "\n\n\n");
            Delay(2);
        }
        return 0;
    }
    
    while (1) {
        char *src = "Hello This is the Parent process\0";
        int cpy_len = strlen(src);
        int len = cpy_len * 60 + 1;
        int i;
        char buf[len];
        for (i = 0; i < len; i += cpy_len) {
            memcpy(buf + i, src, cpy_len);
        }
        buf[i] = '\0';
        TtyPrintf(0, "%s", buf);
        TtyPrintf(0, "\n\n\n");
        Delay(2);
    }
    return 0;
}