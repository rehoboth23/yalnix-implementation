
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    char *src = "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";
    int cpy_len = strlen(src);
    int len = cpy_len * 600;
    int i;
    char buf[len];
    for (i = 0; i < len; i += cpy_len) {
        memcpy(buf + i, src, cpy_len);
    }
}
