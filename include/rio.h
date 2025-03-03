#ifndef RIO_H
#define RIO_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

#define BUFFER_SIZE 8192 // 8 kb buffer size

typedef struct{
    int fd;
    char buffer[BUFFER_SIZE];
    size_t pointer;
    size_t curr_buffer_size;
}rio_buf;


/*
The rio_unbuffered_read function transfers up to read_size bytes from the current file position of descriptor fd to memory location buf

Args
    int fd - file descriptor of the file we're reading from
    void * buf - buffer in which we'll store the read data
    size_t read_size - 
*/
ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size);

ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size);

int rio_init_buffer(int fd, rio_buf * buf);

size_t fill_buffer(rio_buf * buf);
bool is_buffer_empty(rio_buf * buf);
ssize_t rio_buffered_readline(rio_buf * buf, void * user_buf, size_t read_size);

ssize_t rio_buffered_read(rio_buf * buf, void * user_buf, size_t read_size);


#endif