#include "../include/net.h"
#include "../include/logger.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


int open_clientfd(char *hostname, char *port) {
    LOG_INFO("Opening client connection to %s:%s", hostname, port);
    
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME;

    int success_code = getaddrinfo(hostname, port, &hints, &results);
    if (success_code){
        LOG_ERROR("getaddrinfo failed for %s:%s - %s", hostname, port, gai_strerror(success_code));
        return -1;
    }
    
    addrinfo * curr_socket_candidate = results;
    int candidate_counter = 0;
    int client_fd  = -1;
    
    for(; curr_socket_candidate; curr_socket_candidate = curr_socket_candidate->ai_next) {
        candidate_counter += 1;
        LOG_DEBUG("Trying socket candidate %d for %s:%s", candidate_counter, hostname, port);
        
        client_fd = socket(curr_socket_candidate->ai_family, curr_socket_candidate->ai_socktype, curr_socket_candidate->ai_protocol);
        
        if (client_fd == -1) {
            LOG_WARN("Failed to open socket %d: %s", candidate_counter, strerror(errno));
            continue;
        }

        if (connect(client_fd, curr_socket_candidate->ai_addr, curr_socket_candidate->ai_addrlen)) {
            LOG_WARN("Client socket candidate %d failed to connect: %s", candidate_counter, strerror(errno));
            close(client_fd);
            continue;
        }
        
        LOG_DEBUG("Successfully connected socket candidate %d", candidate_counter);
        break;
    }

    if(client_fd == -1) {
        LOG_ERROR("All socket candidates failed for %s:%s", hostname, port);
    } else {
        LOG_INFO("Successfully opened client connection to %s:%s (fd=%d)", hostname, port, client_fd);
    }

    freeaddrinfo(results);
    return client_fd;
}

int open_listenfd(char * port) {
    LOG_INFO("Opening listening socket on port %s", port);
    
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;

    int success_code = getaddrinfo(NULL, port, &hints, &results);
    if(success_code){
        LOG_ERROR("getaddrinfo failed for port %s - %s", port, gai_strerror(success_code));
        return -1;
    }

    addrinfo * curr_socket_candidate = results;
    int candidate_counter = 0;
    int server_fd  = -1;

    for(; curr_socket_candidate; curr_socket_candidate = curr_socket_candidate->ai_next) {
        candidate_counter += 1;
        LOG_DEBUG("Trying socket candidate %d for listen on port %s", candidate_counter, port);
        
        server_fd = socket(curr_socket_candidate->ai_family, curr_socket_candidate->ai_socktype, curr_socket_candidate->ai_protocol);
        
        if (server_fd == -1) {
            LOG_WARN("Failed to open socket %d: %s", candidate_counter, strerror(errno));
            continue;
        }
        
        int optval = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
            LOG_WARN("Failed to set socket options for candidate %d: %s", candidate_counter, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
        if (bind(server_fd, curr_socket_candidate->ai_addr, curr_socket_candidate->ai_addrlen)) {
            LOG_WARN("Server socket candidate %d failed to bind: %s", candidate_counter, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }

        if(listen(server_fd, BACKLOG)){
            LOG_WARN("Server socket candidate %d failed to listen: %s", candidate_counter, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
        
        LOG_DEBUG("Successfully bound and listening on socket candidate %d", candidate_counter);
        break;
    }

    if(server_fd == -1) {
        LOG_ERROR("All socket candidates failed for listening on port %s", port);
    } else {
        LOG_INFO("Successfully opened server listening socket on port %s (fd=%d)", port, server_fd);
    }

    freeaddrinfo(results);
    return server_fd;
}
