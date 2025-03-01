#include "../include/rio.h"
#include <unistd.h> // for read and write
#include <stdio.h>
#include <errno.h>

ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size) {
    /*
    
    */
   char * bufp = (char *) buf;
    while(read_size != 0) {
        size_t bytes_read = read(fd, buf, read_size);
        if(bytes_read == -1) {
            fprintf(stderr, "Read failed. Error : ", strerror(errno));
            return -1;
        }
        if(bytes_read == 0) { 
            break;
        }
        
        read_size -= bytes_read;
        bufp += bytes_read;
    }

    return 0;
}

ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size) {
    while(write_size != 0) {
        size_t bytes_written = write(fd, buf, write_size);
        if(bytes_written == -1) {
            fprintf(stderr, "Write failed. Error : ", str(errno));
            return -1;
        }
        if(bytes_written == 0) { 
            break;
        }
        write_size -= bytes_written;
    }       
    return 0;
}