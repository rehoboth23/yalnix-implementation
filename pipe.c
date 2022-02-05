/*
 *  The format of the file system and other constants used by YFS
 */

#include <ylib.h>
#include "pipe.h"


/* see pipe.h for more information */
int pipe(const int __desc[2]) {

}

/* see pipe.h for more information */
int read(const int __fd, const void *__dest, const size_t __bytes_to_read) {
    // will call pipe
}

/* see pipe.h for more information */
int write(const int __fd, const void *__src, const size_t __bytes_to_read) {

}
