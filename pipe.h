/* function to initialize pipe */
int pipe(const int __desc[2]);

/* function to read bytes from a pipe into a memory location*/
int read(const int __fd, const void *__dest, const size_t __bytes_to_read);

/* function to write bytes from a source into a pipe */
int write(const int __fd, const void *__src, const size_t __bytes_to_read);
