#include "net.h"
#include "rio.h"
#include "http_parser.h"
#include "request_handler.h"
#include "logger.h"
#include "config.h"
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>


int main(int argc, char ** argv)  {
    LOG_INFO("Instantiating Server Configuration");
    server_config config;
    if(!config_load(&config, "../config.ini")) {
        LOG_ERROR("Failed to load server config. Failed to start server");
        return -1;
    }

    if(argc >= 2) {
        LOG_WARN("Extra command line parameters passed are ignored. Please edit the server config file to use any custom parameters");
    }

    int listen_fd = open_listenfd(config.port);
    if(listen_fd == -1) {
        LOG_ERROR("Could not open listening socket on port %s", config.port);
        return -1;
    }

    struct sockaddr_storage client_addr; // ensures that all types of client addresses are compatible
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char buf[BUFFER_SIZE];
    for(int conn_fd = accept(listen_fd, (sockaddr *) &client_addr, &addrlen); ;conn_fd = accept(listen_fd, (sockaddr *) &client_addr, &addrlen)) { 
        if(conn_fd == - 1) {
            fprintf(stderr, "failed to connect to current client. Moving to next client : %s", strerror(errno));
            continue;
        }
        char ip[INET6_ADDRSTRLEN], port[MAX_PORT_STRLEN];
        int ret = getnameinfo(
            &client_addr,
            &addrlen,
            ip,
            INET6_ADDRSTRLEN,
            port,
            MAX_PORT_STRLEN,
            NI_NUMERICHOST | NI_NUMERICSERV
        );

        if(ret != 0) {
            LOG_INFO("Connection established with client. Connected descriptor fd : %d", conn_fd);
            LOG_WARN("getnameinfo error, failed to get ip and port info of client. ERROR : %s", gai_strerror(ret));
        } else {
            LOG_INFO("Connection established with client having socket (ip, port) = (%s, %s). Connected descriptor fd : %d", ip, port, conn_fd);
        } 
        
        // after establishing connection, 
        // 1. read the client request. I should probably send an http error message to the client and then close the connection
        rio_buf buf;
        rio_init_buffer(conn_fd, &buf);
        char http_request[BUFFER_SIZE];
        ssize_t bytes_read = 0;
        char * header_end;
        int flag = 0;
        do {
            bytes_read += rio_buffered_readline(&buf, http_request + bytes_read, BUFFER_SIZE - bytes_read);
            if(bytes_read == -1) {
                LOG_ERROR("Failed to read http request from connected descriptor with fd : %d", conn_fd);
                LOG_INFO("Closing current connection.");
                flag = 1;
                break;
            }
             header_end = strstr(http_request, "\r\n\r\n");
        } while(!header_end || bytes_read == BUFFER_SIZE);


        if(!header_end) { // could not find end of http request
            LOF_ERROR("Malformed HTTP request, does not contain header end.");
            LOG_INFO("Closing current connection");
            flag = 1;
        }

        if(flag) {
            // should probably send some http response message
            close(conn_fd);
            continue;
        }


        // 2. parse the http request
        // If i got here, it is an assertion that there are no pre-parsing errors with the request
        http_request request;
        parse_http_request(http_request, &request, &config);

        // 3. execute the parsed http request
        // 4. Close the client connection
        // 5. repeat.
    }



    return 0;
}
