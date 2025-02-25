#include "../include/utils.h"
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
    hints.ai_flags |= AI_ADDRCONFIG;
    hints.ai_flags |= AI_CANONNAME;

    int success_code = getaddrinfo(hostname, port, &hints, &results);
    if (!success_code){
        fprintf("Unable to connect to server: %s", gai_strerror(success_code));
        return -1;
    }
    
    addrinfo * curr_socket_cadidate = results;
    int candidate_counter = 0;
    int client_fd  = -1;
    
    for(; curr_socket_cadidate; curr_socket_cadidate = curr_socket_cadidate->ai_next) {
        candidate_counter += 1;
        int clientfd = socket(curr_socket_cadidate->ai_family, curr_socket_cadidate->ai_socktype, curr_socket_cadidate->ai_protocol);
        
        if (clientfd == -1) {
            fprintf(stderr, "Failed to open socket %d: %s\n", candidate_counter, strerror(errno));
            continue;
        }

        if (connect(clientfd, curr_socket_cadidate->ai_addr, curr_socket_cadidate->ai_addrlen)) {
            fprintf(stderr, "Client socket candidate %d failed to connect: %s\n", candidate_counter, strerror(errno));
            close(clientfd);
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

    

    return 0;
}