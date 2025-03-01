#ifndef RIO_H
#define RIO_H

#include <sys/types.h>
#include <stddef.h>

/*
The rio_unbuffered_read function transfers up to read_size bytes from the current file position of descriptor fd to memory location buf

Args
    int fd - file descriptor of the file we're reading from
    void * buf - buffer in which we'll store the read data
    size_t read_size - 
*/
ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size);

ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size);
#endif 