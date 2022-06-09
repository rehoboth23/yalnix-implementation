
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

int main(int argc, char const *argv[]) {
    int a = 5;
    int b = 0;
    int crash = a / b;
    return 0;
}
