/*
clang -std=c99 -Wall -Wextra -Werror -g -O0 -I./include \
  src/server.c src/net.c src/rio.c src/http_parser.c src/request_handler.c src/config.c \
  -pthread -lm -o executables/server
*/
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
#include <stdlib.h>
#include <signal.h>

/**
 * Send a simple HTTP error response to the client
 */
void send_error_response(int client_fd, int status_code, const char *reason, const char *message) {
    http_response error_response;
    initialize_response(&error_response);
    
    error_response.status_code = status_code;
    free(error_response.reason);
    error_response.reason = strdup(reason);
    
    // Create simple HTML error page
    char error_body[512];
    snprintf(error_body, sizeof(error_body),
        "<html><head><title>%d %s</title></head>"
        "<body><h1>%d %s</h1><p>%s</p></body></html>",
        status_code, reason, status_code, reason, message);
    
    error_response.content_type = strdup("text/html");
    error_response.content_length = strlen(error_body);
    
    // Generate and send header
    char *header = generate_response_header(&error_response);
    if (header) {
        rio_unbuffered_write(client_fd, header, strlen(header));
        free(header);
    }
    
    // Send body
    rio_unbuffered_write(client_fd, error_body, strlen(error_body));
    
    destroy_response(&error_response);
}

/**
 * Read complete HTTP request from client socket
 * Returns 0 on success, -1 on error
 */
int read_http_request(int client_fd, char *request_buffer, size_t buffer_size) {
    rio_buf rio_buf;
    rio_init_buffer(client_fd, &rio_buf);
    
    size_t total_read = 0;
    char line[BUFFER_SIZE];
    
    // Read request line by line until we find the end of headers
    while (total_read < buffer_size - 1) {
        ssize_t line_length = rio_buffered_readline(&rio_buf, line, BUFFER_SIZE);
        
        if (line_length <= 0) {
            LOG_ERROR("Failed to read HTTP request line");
            return -1;
        }
        
        // Check if adding this line would overflow the buffer
        if (total_read + line_length >= buffer_size) {
            LOG_ERROR("HTTP request too large for buffer");
            return -1;
        }
        
        // Add line to request buffer
        memcpy(request_buffer + total_read, line, line_length);
        total_read += line_length;
        
        // Check if we've reached the end of headers (\r\n\r\n)
        if (total_read >= 4) {
            if (strstr(request_buffer, "\r\n\r\n")) {
                request_buffer[total_read] = '\0';
                LOG_DEBUG("Complete HTTP request read (%zu bytes)", total_read);
                return 0;
            }
        }
        
        // Safety check: if we read just \r\n, this might be the end
        if (line_length == 2 && line[0] == '\r' && line[1] == '\n') {
            request_buffer[total_read] = '\0';
            LOG_DEBUG("Complete HTTP request read (%zu bytes)", total_read);
            return 0;
        }
    }
    
    LOG_ERROR("HTTP request too large or missing header terminator");
    return -1;
}

/**
 * Handle a single client connection
 */
void handle_client(int client_fd, server_config *config) {
    char request_buffer[BUFFER_SIZE * 4]; // 32KB buffer for HTTP request
    
    LOG_INFO("Handling client request on fd %d", client_fd);
    
    // Read the complete HTTP request
    if (read_http_request(client_fd, request_buffer, sizeof(request_buffer)) < 0) {
        LOG_ERROR("Failed to read HTTP request from client");
        send_error_response(client_fd, 400, "Bad Request", 
                          "Malformed HTTP request or request too large");
        return;
    }
    
    LOG_DEBUG("Raw HTTP request: %.200s...", request_buffer);
    
    // Initialize request structure
    http_request request;
    initialize_request(&request);
    
    // Parse the HTTP request
    http_request *parsed_request = parse_http_request(request_buffer, &request, config);
    
    if (parsed_request == NULL) {
        LOG_ERROR("Failed to parse HTTP request");
        send_error_response(client_fd, 400, "Bad Request", 
                          "Invalid HTTP request format");
        destroy_request(&request);
        return;
    }
    
    LOG_INFO("Parsed %s request for path: %s", 
             request.method == GET ? "GET" : "UNKNOWN",
             request.path ? request.path : "NULL");
    
    // Execute the request
    int execution_result = execute_request(parsed_request, client_fd, config);
    
    if (execution_result < 0) {
        LOG_ERROR("Request execution failed");
        // execute_request should have already sent an error response
    } else {
        LOG_INFO("Request executed successfully");
    }
    
    // Cleanup
    destroy_request(&request);
}

/**
 * Signal handler for graceful shutdown
 */
volatile sig_atomic_t server_running = 1;

void signal_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    LOG_INFO("Received shutdown signal, stopping server...");
    server_running = 0;
}

/**
 * Main server function
 */
int main(int argc, char **argv) {
    (void) argv;
    LOG_INFO("Starting Sequential HTTP Server (Stage 2)");
    
    // Set up signal handlers for graceful shutdown
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // No SA_RESTART - force EINTR
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    // Load server configuration
    server_config config;
    config_init(&config);
    
    // Try to load config file from parent directory (as in original code)
    if (!config_load(&config, "../config.ini")) {
        LOG_WARN("Failed to load config from ../config.ini, trying ./config.ini");
        if (!config_load(&config, "./config.ini")) {
            LOG_ERROR("Failed to load server configuration from both ../config.ini and ./config.ini");
            config_cleanup(&config);
            return 1;
        }
    }
    
    if (argc >= 2) {
        LOG_WARN("Extra command line parameters ignored. Edit config.ini to change settings.");
    }
    
    LOG_INFO("Server configuration loaded successfully");
    LOG_INFO("Port: %s", config.port);
    LOG_INFO("Document Root: %s", config.document_root);
    LOG_INFO("Server Name: %s", config.server_name);
    
    // Open listening socket
    int listen_fd = open_listenfd(config.port);
    if (listen_fd < 0) {
        LOG_ERROR("Failed to open listening socket on port %s", config.port);
        config_cleanup(&config);
        return 1;
    }
    
    LOG_INFO("Server listening on port %s (fd=%d)", config.port, listen_fd);
    LOG_INFO("Server ready to accept connections...");
    
    // Main server loop - sequential processing
    while (server_running) {
        struct sockaddr_storage client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        // Accept incoming connection
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            if (errno == EINTR) {
                // Interrupted by signal - check if we should shutdown
                if (!server_running) {
                    LOG_INFO("Accept interrupted, shutting down...");
                    break;
                }
                // Otherwise just retry accept
                continue;
            }
            LOG_ERROR("Failed to accept connection: %s", strerror(errno));
            continue;
        }
        
        // Check shutdown flag even on successful accept
        if (!server_running) {
            LOG_INFO("Shutdown requested, closing new connection");
            close(client_fd);
            break;
        }
        
        // Get client address information
        char client_ip[INET6_ADDRSTRLEN];
        char client_port[MAX_PORT_STRLEN];
        
        int gni_result = getnameinfo(
            (struct sockaddr *)&client_addr, addr_len,
            client_ip, sizeof(client_ip),
            client_port, sizeof(client_port),
            NI_NUMERICHOST | NI_NUMERICSERV
        );
        
        if (gni_result == 0) {
            LOG_INFO("Connection accepted from %s:%s (fd=%d)", client_ip, client_port, client_fd);
        } else {
            LOG_INFO("Connection accepted from unknown client (fd=%d)", client_fd);
            LOG_WARN("getnameinfo failed: %s", gai_strerror(gni_result));
        }
        
        // Handle the client request (sequential processing)
        handle_client(client_fd, &config);
        
        // Close client connection
        if (close(client_fd) < 0) {
            LOG_ERROR("Failed to close client connection (fd=%d): %s", client_fd, strerror(errno));
        } else {
            LOG_INFO("Client connection closed (fd=%d)", client_fd);
        }
    }
    
    // Cleanup and shutdown
    LOG_INFO("Shutting down server...");
    
    if (close(listen_fd) < 0) {
        LOG_ERROR("Failed to close listening socket: %s", strerror(errno));
    }
    
    config_cleanup(&config);
    LOG_INFO("Server shutdown complete");
    
    return 0;
}