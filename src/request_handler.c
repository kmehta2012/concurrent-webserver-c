#include "../include/request_handler.h"
#include "../include/rio.h"
#include "../include/logger.h"
#include <limits.h>  // For PATH_MAX
#include <fcntl.h>   // For open() flags like O_RDONLY
#include <errno.h>
#include <stdlib.h>

char * get_absolute_path(http_request * request, server_config * config) {
    size_t document_root_path_length = strlen(config->document_root);
    size_t requested_file_path_length = strlen(request->path);

    size_t abs_path_len = document_root_path_length + requested_file_path_length; 

    if(abs_path_len + 1 == PATH_MAX) { // also need to store the null terminator
        return NULL;
    }
    
    char * abs_file_path = (char *) malloc(abs_path_len + 1); // +1 for the null terminator
    strcpy(abs_file_path, config->document_root);
    strcat(abs_file_path, request->path);
    return abs_file_path;
}

int execute_request(http_request *request, int client_fd, server_config *config) {
    http_response response;
    initialize_response(&response);
    int status;
    if(request->is_dynamic) {
        status = serve_dynamic(request, client_fd, config);
    }
    else {
        status = serve_static(request, &response, client_fd, config);
    }
    if(status == -1){
        char * response_header = generate_response_header(&response);
        if(rio_unbuffered_write(client_fd, response_header, strlen(response_header)) == -1) {
            return -1;
        }
    }
    return 0; // if the error write failed then just gracefully close the connection without doing anything. Caller will do this. 
}

int serve_static(http_request *request, http_response * response, int client_fd, server_config *config) {

    char * abs_file_path = get_absolute_path(request, config);
    if(!abs_file_path) {
        response->status_code = 414;
        response->reason = "URI Too Long";
        return -1;
    }
    int fd = open(abs_file_path, O_RDONLY);
    if (fd < 0) {
        switch (errno) {
            case ENOENT:
                // File not found
                response->status_code = 404;
                response->reason = "Not Found";
                break;
            case EACCES:
                // Permission denied
                response->status_code = 403;
                response->reason = "Forbidden";
                break;
            case EMFILE:
            case ENFILE:
                // Too many open files
                response->status_code = 503;
                response->reason = "Service Unavailable";
                break;
            default:
                // Any other error
                response->status_code = 500;
                response->reason = "Internal Server Error";
                LOG_ERROR("Failed to open file %s: %s", abs_file_path, strerror(errno));
                break;
        }
        free(abs_file_path);
        return -1;
    }
    free(abs_file_path);

    response->status_code = 200;
    response->reason = "OK";
    char * response_header = generate_response_header(response);

    // Commit to the response header even if the read/write from/to file/socket fail.

    if(rio_unbuffered_write(client_fd, response_header, strlen(response_header)) == -1) {
        response->status_code = 500;
        response->reason = "Internal Server Error";
        return -1;
    }

    char read_buffer[BUFFER_SIZE];
    ssize_t read_size = 0;
    do{
        read_size = rio_unbuffered_read(fd, read_buffer, BUFFER_SIZE);
        if(read_size < 0 || rio_unbuffered_write(client_fd, read_buffer, read_size) == -1) {
            response->status_code = 500;
            response->reason = "Internal Server Error";
            return -1;
        }
    }while(read_size > 0);

    return 0;
}

