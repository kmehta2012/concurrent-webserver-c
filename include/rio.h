#ifndef RIO_H
#define RIO_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

#define BUFFER_SIZE 8192 // 8 kb buffer size

typedef struct{
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t pointer; // position of the byte starting which we'll read or write
    ssize_t curr_buffer_size; 
}rio_buf;


/*
Transfers up to read_size bytes from the current file position of descriptor fd to memory location buf

Args
    int fd - file descriptor of the file we're reading from
    void * buf - buffer in which we'll store the read data
    size_t read_size - max bytes to read
Returns
    number of bytes returned on success, -1 on failure
*/
ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size);



/*
Transfers up to write_size bytes from buf to fd

Args
    int fd - file descriptor of the file we're writing to
    void * buf - buffer whose data we're writing
    size_t write_size - max bytes to read
Returns
    number of bytes written on success, -1 on failure
*/
ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size);


/*
associates buffer buf with fd. No data is read into the buffer here.

Sets buf->fd = fd;
     buf->curr_buffer_size = 0;
     buf->pointer = 0;
Args
    int fd - file descriptor of the file we're reading from
    rio_buf * buf - buffer in which we'll store the read data
Returns
    number of bytes written to buf on success, -1 on failure
*/
int rio_init_buffer(int fd, rio_buf * buf);

/*
fills buffer buf with atmost BUFFER_SIZE bytes of data from buf->fd

Args
    rio_buf * buf - buffer in which we're filling
Returns
    number of bytes written to buf on success, -1 on failure
*/
ssize_t fill_buffer(rio_buf * buf);

/*
returns true if the buffer pointer is at the last position of the buffer. 
Args
    rio_buf * buf - buffer in question
Returns
    true if empty false otherwise
*/
static inline bool is_buffer_empty(rio_buf * buf) {
    return buf->pointer == buf->curr_buffer_size ? true : false;
}


/*
fills user_buf with atmost read_size - 1 bytes of null terminated text data from buffer buf. Reads until read_size - 1 bytes are read or a \n is read. 
\0 is not considered part of the data for calculating read size
Args
    rio_buf * buf - buffer from which we're reading
    void * user_buf - buffer to which we're reading
Returns
    -1 on failure, total bytes read on success
*/
ssize_t rio_buffered_readline(rio_buf * buf, void * user_buf, size_t read_size);

/*
buffered version of rio_buffered_read
*/
ssize_t rio_buffered_readb(rio_buf * buf, void * user_buf, size_t read_size);




#endif