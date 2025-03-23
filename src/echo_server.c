#include "../include/net.h"
#include "../include/rio.h"
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int echo(int fd, char * buf, ssize_t write_size) {
    int echo_status = rio_unbuffered_write(fd, buf, write_size);

    if(echo_status == -1) {
        return -1;
    }
    return 0;
}

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 2) {
        fprintf(stderr, "Please run as ./prog_name port_number");
        return -1;
    }
    
    char * port = argv[1];
    int listenfd =  open_listenfd(port);
    struct sockaddr_storage client_addr; // ensures that all types of client addresses are compatible
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char buf[BUFFER_SIZE];

    
    for(int conn_fd = accept(listenfd, (sockaddr *)&client_addr, &addrlen); ;conn_fd = accept(listenfd, (sockaddr *) &client_addr, &addrlen)) {
        if(conn_fd == - 1) {
            fprintf(stderr, "failed to connect to current client. Moving to next client : %s", strerror(errno));
            continue;
        }
        rio_buf server_socket_buf;
        rio_init_buffer(conn_fd, &server_socket_buf);
        for(ssize_t total_bytes = rio_buffered_readline(&server_socket_buf, buf, BUFFER_SIZE); total_bytes != 0; total_bytes = rio_buffered_readline(&server_socket_buf, buf, BUFFER_SIZE)) { // read from connfd
            int echo_status = echo(conn_fd, buf, total_bytes); // echo the same to connfd
            if(echo_status == -1) {
                fprintf(stderr, "failed to echo to client. Closing current connection and moving to the next client");
                close(conn_fd);
                break;
            }
        }
        close(conn_fd);
    }  
    
    return 0;
}
