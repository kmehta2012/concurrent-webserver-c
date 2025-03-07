#include "../include/net.h"
#include "../include/rio.h"
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

int echo(int fd, char * buf) {
    int echo_status = rio_unbuffered_write(fd, buf, BUFFER_SIZE);

    if(echo_status == -1) {
        return -1;
    }
    return 0;
}

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 2) {
        fprintf(stderr, "Please run as ./prog_name port_number");
    }
    char * port = argv[1];
    int listenfd =  open_listenfd(port);
    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char buf[BUFFER_SIZE];
    for(int conn_fd = accept(listenfd, &client_addr, &addrlen); ;conn_fd = accept(listenfd, &client_addr, &addrlen)) {
        if(conn_fd == - 1) {
            fprintf(stderr, "failed to connect to current client : %s", strerror(errno));
            continue;
        }
        rio_unbuffered_read(conn_fd, buf, BUFFER_SIZE);
        int echo_status = echo(conn_fd, buf);
        if(echo_status == -1) {
            fprintf(stderr, "failed to echo to client");
        }
    }
    
    return 0;
}
