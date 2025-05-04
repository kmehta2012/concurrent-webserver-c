#include "rio.h"
#include "utils.h"
#include "logger.h"
#include <unistd.h> // for read and write
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

ssize_t rio_unbuffered_read(int fd, void * buf, size_t read_size) {
   if(read_size > SSIZE_MAX){
        LOG_ERROR("Cannot read more than %zd bytes at once", SSIZE_MAX);
        return -1;
   }
   LOG_DEBUG("Starting unbuffered read from fd %d, requesting %zu bytes", fd, read_size);
   size_t total_bytes_read = 0;
   char * bufp = (char *) buf;
    while(read_size != 0) {
        ssize_t bytes_read = read(fd, bufp, read_size);
        if(bytes_read == -1 && errno == EINTR) {
            LOG_DEBUG("Read interrupted by signal, retrying");
            continue;
        }
        else if(bytes_read == -1) {
            LOG_ERROR("Read failed on fd %d. Error: %s", fd, strerror(errno));
            return -1;
        }
        else if(bytes_read == 0) {  // EOF
            LOG_DEBUG("Reached EOF on fd %d after reading %zu bytes", fd, total_bytes_read);
            break;
        }
        read_size -= bytes_read;
        total_bytes_read += bytes_read;
        bufp += bytes_read;
        LOG_DEBUG("Read %zd bytes from fd %d, %zu bytes remaining", bytes_read, fd, read_size);
    }

    LOG_DEBUG("Completed unbuffered read from fd %d, total bytes read: %zu", fd, total_bytes_read);
    return total_bytes_read;
}

ssize_t rio_unbuffered_write(int fd, void * buf, size_t write_size) {
    if(write_size > SSIZE_MAX){
        LOG_ERROR("Cannot write more than %zd bytes at once", SSIZE_MAX);
        return -1;
   }
    LOG_DEBUG("Starting unbuffered write to fd %d, requesting %zu bytes", fd, write_size);
    char * bufp = (char *) buf;
    size_t total_bytes_written = 0;
    while(write_size != 0) {
        ssize_t bytes_written = write(fd, bufp, write_size);
        if(bytes_written == -1 && errno == EINTR) {
            LOG_DEBUG("Write interrupted by signal, retrying");
            continue;
        }
        else if(bytes_written == -1) { 
            LOG_ERROR("Write failed on fd %d. Error: %s", fd, strerror(errno));
            return -1;
        }
        else if(bytes_written == 0) { 
            LOG_WARN("Zero bytes written to fd %d, possibly socket closed", fd);
            break;
        }
        write_size -= bytes_written;
        total_bytes_written += bytes_written;
        bufp += bytes_written;
        LOG_DEBUG("Wrote %zd bytes to fd %d, %zu bytes remaining", bytes_written, fd, write_size);
    }
    LOG_DEBUG("Completed unbuffered write to fd %d, total bytes written: %zu", fd, total_bytes_written);
    return total_bytes_written;
}

int rio_init_buffer(int fd, rio_buf *buf) {
    LOG_DEBUG("Initializing buffer structure for fd %d without reading data", fd);
    
    buf->fd = fd;
    buf->curr_buffer_size = 0;
    buf->pointer = 0;
    
    LOG_DEBUG("Buffer structure initialized for fd %d (empty, will be filled on first read)", fd);
    return 0;
}

ssize_t fill_buffer(rio_buf * buf) {
    LOG_DEBUG("Filling buffer for fd %d", buf->fd);
    
    ssize_t bytes_read = read(buf->fd, buf->buffer, BUFFER_SIZE);
    
    if(bytes_read < 0) {
        if(errno == EINTR) {
            LOG_DEBUG("Read interrupted by signal, retrying");
            return fill_buffer(buf);  // Retry if interrupted by signal
        } else {
            LOG_ERROR("Failed to fill buffer for fd %d: %s", buf->fd, strerror(errno));
            buf->curr_buffer_size = 0;
            buf->pointer = 0;
            return -1;
        }
    }
    else if(bytes_read == 0) {
        LOG_DEBUG("Reached EOF when filling buffer for fd %d", buf->fd);
    }
    
    buf->curr_buffer_size = bytes_read;
    buf->pointer = 0;
    LOG_DEBUG("Buffer filled for fd %d with %zd bytes", buf->fd, buf->curr_buffer_size);
    return buf->curr_buffer_size;
}

ssize_t rio_buffered_readline(rio_buf * buf, void * user_buf, size_t read_size) {
    LOG_DEBUG("Starting buffered readline from fd %d, max size: %zu", buf->fd, read_size);
    char * user_bufp = (char *) user_buf;
    size_t total_bytes_read = 0;
    
    while(total_bytes_read < read_size-1) {
        if(is_buffer_empty(buf)) {
            LOG_DEBUG("Buffer empty for fd %d, refilling", buf->fd);
            ssize_t fill_size = fill_buffer(buf);
            if(fill_size == -1) {
                LOG_ERROR("Failed to refill buffer for fd %d during readline", buf->fd);
                return -1;
            }
            else if(fill_size == 0) { // Reached EOF
                LOG_DEBUG("Reached EOF for fd %d during readline after %zu bytes", buf->fd, total_bytes_read);
                break;
            }
        }
        char current_char = buf->buffer[buf->pointer];
        *user_bufp = current_char;
        ++user_bufp;
        ++buf->pointer;
        total_bytes_read += 1;

        if(current_char == '\n'){ 
            LOG_DEBUG("Newline found, terminating readline for fd %d after %zu bytes", buf->fd, total_bytes_read);
            break;
        }
    }
    *user_bufp = '\0';
    LOG_DEBUG("Completed buffered readline from fd %d, bytes read: %zu", buf->fd, total_bytes_read);
    return total_bytes_read;
}

ssize_t rio_buffered_readb(rio_buf * buf, void * user_buf, size_t read_size) {
    LOG_DEBUG("Starting buffered read from fd %d, requested size: %zu", buf->fd, read_size);
    char * user_bufp = (char *) user_buf;
    size_t total_bytes_read = 0;
    
    while(total_bytes_read < read_size) {
        if(is_buffer_empty(buf)) {
            LOG_DEBUG("Buffer empty for fd %d, refilling", buf->fd);
            ssize_t fill_size = fill_buffer(buf);
            if(fill_size == -1) {
                LOG_ERROR("Failed to refill buffer for fd %d during read", buf->fd);
                return -1;
            }
            else if(fill_size == 0) { // Reached EOF
                LOG_DEBUG("Reached EOF for fd %d during read after %zu bytes", buf->fd, total_bytes_read);
                break;
            }
        }
        char current_char = buf->buffer[buf->pointer];
        *user_bufp = current_char;
        ++user_bufp;
        ++buf->pointer;
        total_bytes_read += 1;
    }
    LOG_DEBUG("Completed buffered read from fd %d, bytes read: %zu", buf->fd, total_bytes_read);
    return total_bytes_read;
}
