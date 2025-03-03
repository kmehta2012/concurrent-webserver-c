#include "../include/rio.h"
#include "../include/utils.h"
#include <unistd.h> // for read and write
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size) {
    /*
    
    */
   if(read_size > SSIZE_MAX){
        fprintf(stderr, "Cannot read more than %zd bytes at once", SSIZE_MAX);
        return -1;
   }
   size_t total_bytes_read = 0;
   char * bufp = (char *) buf;
    while(read_size != 0) {
        ssize_t bytes_read = read(fd, bufp, read_size);
        if(bytes_read == -1 && errno == EINTR) {
            continue;
        }
        else if(bytes_read == -1) {
            fprintf(stderr, "Read failed. Error : ", strerror(errno));
            return -1;
        }
        else if(bytes_read == 0) {  // EOF
            break;
        }
        
        read_size -= bytes_read;
        total_bytes_read += bytes_read;
        bufp += bytes_read;
    }

    return total_bytes_read;
}

ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size) {
    if(write_size > SSIZE_MAX){
        fprintf(stderr, "Cannot write more than %zd bytes at once", SSIZE_MAX);
        return -1;
   }
    char * bufp = (char *) buf;
    size_t total_bytes_written = 0;
    while(write_size != 0) {
        ssize_t bytes_written = write(fd, bufp, write_size);
        if(bytes_written == -1 && errno == EINTR) {
            continue;
        }
        else if(bytes_written == -1) { 
            fprintf(stderr, "Write failed. Error : ", strerror(errno));
            return -1;
        }
        else if(bytes_written == 0) { 
            break;
        }
        write_size -= bytes_written;
        total_bytes_written += bytes_written;
        bufp += bytes_written;
    }       
    return total_bytes_written;
}
int rio_init_buffer(int fd, rio_buf * buf) {
    buf->curr_buffer_size = rio_unbuffered_read(fd, buf->buffer, BUFFER_SIZE);
    
    if(buf->curr_buffer_size == -1) {
        fprintf(stderr, "rio_unbuffered_read failed to read into buffer. Buffer not initialized");
        buf->curr_buffer_size = 0;
        buf->pointer = buf->curr_buffer_size;
        return -1;
    }
    
    buf->fd = fd;
    buf->pointer = 0;
    return buf->curr_buffer_size;
}

inline bool is_buffer_empty(rio_buf * buf) {
    return buf->pointer == buf->curr_buffer_size ? true : false;
}

ssize_t fill_buffer(rio_buf * buf) {
    buf->curr_buffer_size = rio_unbuffered_read(buf->fd, buf->buffer, BUFFER_SIZE);
    if(buf->curr_buffer_size == -1) {
        fprintf(stderr, "rio_unbuffered_read failed to read into buffer");
        buf->curr_buffer_size = 0;
        buf->pointer = buf->curr_buffer_size;
        return -1;
    }
    buf->pointer = 0;
    return buf->curr_buffer_size;
}

ssize_t rio_buffered_readline(rio_buf * buf, void * user_buf, size_t read_size) {
    char * user_bufp = (char *) user_buf;
    size_t total_bytes_read = 0;
    
    while(total_bytes_read < read_size) {
        if(is_buffer_empty(buf)) {
            size_t fill_size = fill_buffer(buf);
            if(fill_size == -1) {
                fprintf(stderr, "fill_buffer failed to fill the buffer");
                return -1;
            }
            else if(fill_size == 0) { // Reached EOF
                break;
            }
        }
        *user_bufp = buf->buffer[buf->pointer];
        ++user_bufp;
        ++buf->pointer;
        total_bytes_read += 1;

        if(*user_bufp == '\n'){ 
            break;
        }
    }
    return total_bytes_read;
}

