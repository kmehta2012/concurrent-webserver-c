#include "../include/rio.h"
#include <unistd.h> // for read and write
#include <stdio.h>
#include <errno.h>
#include <limits.h>

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
        else if(bytes_read == 0) { 
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
