#ifndef __PIPE_H_
#define ___PIPE_H_

/* function to initialize pipe */
int nala_pipe(const int __desc[2]);

/* function to read bytes from a pipe into a memory location*/
int nala_read(const int __fd, const void *__dest, const size_t __bytes_to_read);

/* function to write bytes from a source into a pipe */
int nala_write(const int __fd, const void *__src, const size_t __bytes_to_read);

#endif
