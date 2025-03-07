#include "../include/net.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


int open_clientfd(char *hostname, char *port) {
    
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME;

    int success_code = getaddrinfo(hostname, port, &hints, &results);
    if (success_code){
        fprintf(stderr, "Unable to connect to server: %s", gai_strerror(success_code));
        return -1;
    }
    
    addrinfo * curr_socket_candidate = results;
    int candidate_counter = 0;
    int client_fd  = -1;
    
    for(; curr_socket_candidate; curr_socket_candidate = curr_socket_candidate->ai_next) {
        candidate_counter += 1;
        client_fd = socket(curr_socket_candidate->ai_family, curr_socket_candidate->ai_socktype, curr_socket_candidate->ai_protocol);
        
        if (client_fd == -1) {
            fprintf(stderr, "Failed to open socket %d: %s\n", candidate_counter, strerror(errno));
            continue;
        }

        if (connect(client_fd, curr_socket_candidate->ai_addr, curr_socket_candidate->ai_addrlen)) {
            fprintf(stderr, "Client socket candidate %d failed to connect: %s\n", candidate_counter, strerror(errno));
            close(client_fd);
            continue;
        }
        break;
    }

    if(client_fd == -1) {
        fprintf(stderr, "Unable to connect to server : All candidate sockets failed");
    }

    freeaddrinfo(results);
    return client_fd;
}

int open_listenfd(char * port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;

    int success_code = getaddrinfo(NULL, port, &hints, &results);
    if(success_code){
        fprintf(stderr, "Unable to connect to server : %s", gai_strerror(success_code));
    }

    addrinfo * curr_socket_candidate = results;
    int candidate_counter = 0;
    int server_fd  = -1;

    for(; curr_socket_candidate; curr_socket_candidate = curr_socket_candidate->ai_next) {
        candidate_counter += 1;
        server_fd = socket(curr_socket_candidate->ai_family, curr_socket_candidate->ai_socktype, curr_socket_candidate->ai_protocol);
        
        if (server_fd == -1) {
            fprintf(stderr, "Failed to open socket %d: %s\n", candidate_counter, strerror(errno));
            continue;
        }
        
        int optval = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
            fprintf(stderr, "Failed to set socket options: %s\n", strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
        if (bind(server_fd, curr_socket_candidate->ai_addr, curr_socket_candidate->ai_addrlen)) {
            fprintf(stderr, "Server socket candidate %d failed to connect: %s\n", candidate_counter, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }

        if(listen(server_fd, BACKLOG)){
            fprintf(stderr, "Server socket candidate %d failed to convert to listening socket: %s\n", candidate_counter, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
        break;
    }

    if(server_fd == -1) {
        fprintf(stderr, "Unable to open listening socket : All candidate sockets failed");
    }

    freeaddrinfo(results);
    return server_fd;
}