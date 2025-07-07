#include "request_handler.h"
#include "rio.h"
#include "logger.h"
#include <limits.h>  // For PATH_MAX
#include <fcntl.h>   // For open() flags like O_RDONLY
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>


extern char **environ;  // Declaration of the global environ variable

void initialize_response(http_response *response) {
    if (!response) {
        LOG_ERROR("NULL response passed to initialize_response");
        return;
    }
    
    // Set status information to default values
    response->status_code = 200;  // Default to OK
    response->reason = "OK";
    
    // Set standard headers to NULL - they'll be set as needed
    response->server = "TuringBolt/0.1"; // perhaps I'll figure out a better name eventually
    
    // Generate current date in HTTP format
    time_t now = time(NULL);
    
    struct tm tm_info;
    gmtime_r(&now, &tm_info);
    
    char date_buf[64];
    strftime(date_buf, sizeof(date_buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_info);
    response->date = strdup(date_buf);  // Allocate and copy date string
    
    // Initialize content-related fields
    response->content_type = NULL;
    response->content_length = 0;
    response->content_encoding = NULL;
    response->last_modified = NULL;
    
    // Set connection management
    response->connection = "close";  // Default to closing connection
    
    // Initialize caching fields
    response->cache_control = NULL;
    response->etag = NULL;
    
    // Initialize body fields
    response->body = NULL;
    response->is_file = false;
    
    // Initialize extra headers array
    response->extra_header_names = NULL;
    response->extra_header_values = NULL;
    response->extra_header_count = 0;
}

char * get_absolute_path(http_request * request, server_config * config) {
    size_t document_root_path_length = strlen(config->document_root);
    size_t requested_file_path_length = strlen(request->path);
    
    // Skip the leading slash in request path if document_root ends with slash
    size_t skip_slash = (document_root_path_length > 0 && 
                      config->document_root[document_root_path_length-1] == '/' && 
                      request->path[0] == '/') ? 1 : 0;
    
    size_t abs_path_len = document_root_path_length + requested_file_path_length - skip_slash;

    if(abs_path_len + 1 >= PATH_MAX) { // +1 for null terminator
        return NULL;
    }
    
    char * abs_file_path = (char *) malloc(abs_path_len + 1);
    strcpy(abs_file_path, config->document_root);
    
    // Concatenate path, skipping leading slash if needed
    strcat(abs_file_path, request->path + skip_slash);
    
    return abs_file_path;
}
/**
 * Converts MIME_TYPE enum to corresponding Content-Type string
 * 
 * Args:
 *    MIME_TYPE mime_type: The MIME type enum value
 * 
 * Returns:
 *    const char*: String representation of the Content-Type
 */
static char* mime_type_to_string(MIME_TYPE mime_type) {
    switch (mime_type) {
        case TEXT_HTML:
            return "text/html";
        case TEXT_PLAIN:
            return "text/plain";
        case TEXT_CSS:
            return "text/css";
        case APPLICATION_JAVASCRIPT:
            return "application/javascript";
        case APPLICATION_JSON:
            return "application/json";
        case APPLICATION_XML:
            return "application/xml";
        case APPLICATION_PDF:
            return "application/pdf";
        case APPLICATION_ZIP:
            return "application/zip";
        case APPLICATION_POSTSCRIPT:
            return "application/postscript";
        case IMAGE_GIF:
            return "image/gif";
        case IMAGE_PNG:
            return "image/png";
        case IMAGE_JPEG:
            return "image/jpeg";
        case IMAGE_SVG:
            return "image/svg+xml";
        case AUDIO_MPEG:
            return "audio/mpeg";
        case AUDIO_WAV:
            return "audio/wav";
        case VIDEO_MP4:
            return "video/mp4";
        case VIDEO_WEBM:
            return "video/webm";
        case FONT_WOFF:
            return "font/woff";
        case FONT_WOFF2:
            return "font/woff2";
        default:
            return "application/octet-stream";  // Default binary type
    }
}


const char *get_reason_phrase(int code) {
    switch (code) {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        case 500: return "Internal Server Error";
        default:  return "Unknown Status Code";
    }
}

int set_content_headers(int fd, http_request *request, http_response *response, const char *file_path) {
    if (!response || fd < 0 || !request) {
        LOG_ERROR("Invalid parameters passed to set_content_headers");
        return -1;
    }
    
    // Get file information including size and modification time
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        LOG_ERROR("Failed to get file stats for %s: %s", file_path, strerror(errno));
        return -1;
    }
    
    // Set Content-Length based on file size
    response->content_length = (size_t) file_stat.st_size;
    
    // Set Content-Type based on MIME type from request
    response->content_type = mime_type_to_string(request->mime_type);
    
    // Set Last-Modified header
    struct tm *tm_info = gmtime(&file_stat.st_mtime);
    char last_mod_buf[64];
    strftime(last_mod_buf, sizeof(last_mod_buf), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
    
    // Free existing value if present
    if (response->last_modified) {
        free(response->last_modified);
    }
    
    response->last_modified = strdup(last_mod_buf);
    if (!response->last_modified) {
        LOG_ERROR("Failed to allocate memory for Last-Modified header");
        return -1;
    }
    
    // Set Content-Encoding (default is NULL, meaning no encoding). will add support for this later if i want to. 
    response->content_encoding = NULL;
    
    return 0;
}

int execute_request(http_request *request, int client_fd, server_config *config) {
    http_response response;
    initialize_response(&response);
    int status;
    
    if(request->is_dynamic) {
        status = serve_dynamic(request, &response, client_fd, config);
    }
    else {
        status = serve_static(request, &response, client_fd, config);
    }
    
    if(status == -1){
        char * response_header = generate_response_header(&response);
        if(response_header) {
            if(rio_unbuffered_write(client_fd, response_header, strlen(response_header)) == -1) {
                LOG_ERROR("Failed to write error response header");
            }
            free(response_header);
        }
    }
    
    destroy_response(&response);
    return 0; 
}

int serve_static(http_request *request, http_response * response, int client_fd, server_config *config) {
    if (!request || !response || !config || client_fd < 0) {
        LOG_ERROR("Invalid parameters passed to serve_dynamic");
        response->status_code = 500;
        response->reason = "Internal Server Error";
        return -1;
    }
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
    set_content_headers(fd, request, response, abs_file_path);
    free(abs_file_path);

    response->status_code = 200;
    response->reason = "OK";

    
    char * response_header = generate_response_header(response);

    if(!response_header) {
        LOG_ERROR("Error in generating response header");
        response->status_code = 500;
        response->reason = "Internal Server Error";
        close(fd);
        return -1;
    }

    // Commit to the response header even if the read/write from/to file/socket fail.

    if(rio_unbuffered_write(client_fd, response_header, strlen(response_header)) == -1) {
        response->status_code = 500;
        response->reason = "Internal Server Error";
        free(response_header);
        return -1;
    }
    free(response_header);

    char read_buffer[BUFFER_SIZE];
    ssize_t read_size = 0;
    do{
        read_size = rio_unbuffered_read(fd, read_buffer, BUFFER_SIZE);
        if(read_size < 0 || rio_unbuffered_write(client_fd, read_buffer, (size_t) read_size) == -1) { // The explicit type cast is useless here but doing it to bypass the compilation flags
            response->status_code = 500; 
            response->reason = "Internal Server Error";
            close(fd);
            return -1;
        }
    }while(read_size > 0);
    
    close(fd);
    return 0;
}

int serve_dynamic(http_request *request, http_response *response, int client_fd, server_config *config) {
    if (!request || !response || !config || client_fd < 0) {
        LOG_ERROR("Invalid parameters passed to serve_dynamic");
        response->status_code = 500;
        response->reason = "Internal Server Error";
        return -1;
    }

    char *abs_file_path = get_absolute_path(request, config);
    if (!abs_file_path) {
        LOG_ERROR("Failed to get absolute path for CGI script");
        response->status_code = 414;
        response->reason = "URI Too Long";
        return -1;
    }

    // Check if file exists and is executable
    if (access(abs_file_path, F_OK) != 0) {
        LOG_ERROR("CGI script not found: %s", abs_file_path);
        response->status_code = 404;
        response->reason = "Not Found";
        free(abs_file_path);
        return -1;
    }

    if (access(abs_file_path, X_OK) != 0) {
        LOG_ERROR("CGI script not executable: %s", abs_file_path);
        response->status_code = 403;
        response->reason = "Forbidden";
        free(abs_file_path);
        return -1;
    }

    // Create pipes for communication with CGI script
    int pipe_to_child[2];   // Server writes to child (for POST data later)
    int pipe_from_child[2]; // Child writes to server (CGI output)
    
    if (pipe(pipe_to_child) < 0 || pipe(pipe_from_child) < 0) {
        LOG_ERROR("Failed to create pipes for CGI communication: %s", strerror(errno));
        response->status_code = 500;
        response->reason = "Internal Server Error";
        free(abs_file_path);
        return -1;
    }

    LOG_INFO("Executing CGI script: %s", abs_file_path);

    pid_t pid = fork();
    if (pid < 0) {
        LOG_ERROR("Failed to fork for CGI execution: %s", strerror(errno));
        response->status_code = 500;
        response->reason = "Internal Server Error";
        close(pipe_to_child[0]);
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        close(pipe_from_child[1]);
        free(abs_file_path);
        return -1;
    } 
    else if (pid == 0) {
        // Child process - execute CGI script
        
        // Set up environment variables
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("SERVER_PORT", config->port, 1);
        setenv("SERVER_NAME", config->server_name, 1);
        setenv("SCRIPT_NAME", request->path, 1);
        setenv("SERVER_SOFTWARE", config->server_name, 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("CONTENT_TYPE", "", 1);         // Empty for GET
        setenv("CONTENT_LENGTH", "0", 1);      // 0 for GET
        
        // Set QUERY_STRING if we have parameters
        if (request->param_count > 0) {
            char query_string[BUFFER_SIZE] = "";
            size_t offset = 0;
            for (int i = 0; i < request->param_count; i++) {
                int written = snprintf(query_string + offset, BUFFER_SIZE - offset, 
                                    "%s%s=%s", 
                                    i > 0 ? "&" : "", 
                                    request->param_names[i], 
                                    request->param_values[i]);
                if (written < 0 || offset + written >= BUFFER_SIZE) {
                    // Exit with specific code for query string too long
                    exit(EXIT_QUERY_TOO_LONG);  
                }
                offset += written;
            }
            setenv("QUERY_STRING", query_string, 1);
        } else setenv("QUERY_STRING", "", 1);
        

        // Redirect stdin and stdout for CGI communication
        dup2(pipe_to_child[0], STDIN_FILENO);     // Child reads from server
        dup2(pipe_from_child[1], STDOUT_FILENO);  // Child writes to server
        dup2(pipe_from_child[1], STDERR_FILENO);  // Redirect stderr to same pipe

        // Close all pipe ends in child
        close(pipe_to_child[0]);
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        close(pipe_from_child[1]);

        // Execute the CGI script
        char *argv[] = {abs_file_path, NULL};
        execve(abs_file_path, argv, environ);
        
        // If we get here, exec failed
        fprintf(stderr, "Content-Type: text/plain\r\n\r\nCGI execution failed\r\n");
        exit(1);
    } 
    else {
        // Parent process - read CGI output and send to client
        
        // Close unused pipe ends
        close(pipe_to_child[0]);   // We don't read from child's stdin
        close(pipe_to_child[1]);   // We don't write to child's stdin (for GET)
        close(pipe_from_child[1]); // We don't write to child's stdout
        
        free(abs_file_path);

        // Read all CGI output first
        char *cgi_output = malloc(BUFFER_SIZE * 10); // Start with 80KB buffer
        if (!cgi_output) {
            LOG_ERROR("Failed to allocate memory for CGI output");
            response->status_code = 500;
            response->reason = "Internal Server Error";
            close(pipe_from_child[0]);
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            return -1;
        }

        size_t total_output = 0;
        size_t output_capacity = BUFFER_SIZE * 10;
        char read_buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(pipe_from_child[0], read_buffer, BUFFER_SIZE)) != 0) {
            if (bytes_read < 0) {
                if (errno == EINTR) continue;  // Just retry the read
                
                // Real error
                LOG_ERROR("Failed to read from CGI output: %s", strerror(errno));
                free(cgi_output);
                response->status_code = 500;
                response->reason = "Internal Server Error";
                close(pipe_from_child[0]);
                kill(pid, SIGTERM);
                waitpid(pid, NULL, 0);
                return -1;
            }
            
            // Normal processing for bytes_read > 0
            // Resize buffer if needed
            if (total_output + bytes_read >= output_capacity) {
                output_capacity *= 2;
                char *new_output = realloc(cgi_output, output_capacity);
                if (!new_output) {
                    LOG_ERROR("Failed to reallocate memory for CGI output");
                    free(cgi_output);
                    response->status_code = 500;
                    response->reason = "Internal Server Error";
                    close(pipe_from_child[0]);
                    kill(pid, SIGTERM);
                    waitpid(pid, NULL, 0);
                    return -1;
                }
                cgi_output = new_output;
            }

            memcpy(cgi_output + total_output, read_buffer, bytes_read);
            total_output += bytes_read;
        }

        close(pipe_from_child[0]);

        // Wait for child process to complete
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            LOG_ERROR("Failed to wait for CGI process: %s", strerror(errno));
        }

        if(!WIFEXITED(status)) {
            LOG_ERROR("CGI script failed with status: %d", WEXITSTATUS(status));
            response->status_code = 500;
            response->reason = "Internal Server Error";
            free(cgi_output);
            return -1;            
        }
        int exit_code = WEXITSTATUS(status);
        if (exit_code == EXIT_QUERY_TOO_LONG) {
            LOG_ERROR("CGI script failed: Query string too long");
            response->status_code = 414;  // URI Too Long
            response->reason = "URI Too Long";
            free(cgi_output);
            return -1;
        } else if (exit_code != 0) {
            LOG_ERROR("CGI script failed with exit code: %d", exit_code);
            response->status_code = 500;
            response->reason = "Internal Server Error";
            free(cgi_output);
            return -1;
        }

        if (total_output == 0) {
            LOG_ERROR("CGI script produced no output");
            response->status_code = 500;
            response->reason = "Internal Server Error";
            free(cgi_output);
            return -1;
        }

        // Null-terminate the output
        cgi_output[total_output] = '\0';

        // Find the end of headers (empty line)
        char *header_end = strstr(cgi_output, "\r\n\r\n");
        if (!header_end) {
            header_end = strstr(cgi_output, "\n\n");
        }

        if (!header_end) {
            LOG_ERROR("CGI output missing header/body separator");
            response->status_code = 500;
            response->reason = "Internal Server Error";
            free(cgi_output);
            return -1;
        }

        // Separate headers and body
        char *headers_section;
        char *body_section;
        size_t header_len;
        
        if (strncmp(header_end, "\r\n\r\n", 4) == 0) {
            header_len = header_end - cgi_output;
            body_section = header_end + 4;
        } else {
            header_len = header_end - cgi_output;
            body_section = header_end + 2;
        }

        // Extract headers
        headers_section = malloc(header_len + 1);
        if (!headers_section) {
            LOG_ERROR("Failed to allocate memory for headers");
            response->status_code = 500;
            response->reason = "Internal Server Error";
            free(cgi_output);
            return -1;
        }
        
        memcpy(headers_section, cgi_output, header_len);
        headers_section[header_len] = '\0';

        // Parse and send HTTP response
        int cgi_status = 200;  // Default status
        char status_line[128];
        
        // Look for Status header in CGI output
        char *status_header = strstr(headers_section, "Status:");
        if (status_header) {
            if (sscanf(status_header, "Status: %d", &cgi_status) != 1) {
                cgi_status = 200;  // Default if parsing fails
            }
        }

        // Write HTTP status line
        const char *reason_phrase = get_reason_phrase(cgi_status);
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", cgi_status, reason_phrase);
        
        if (rio_unbuffered_write(client_fd, status_line, strlen(status_line)) == -1) {
            LOG_ERROR("Failed to write status line to client");
            free(headers_section);
            free(cgi_output);
            return -1;
        }

        // Write standard server headers
        char server_headers[256];
        snprintf(server_headers, sizeof(server_headers), 
                "Server: %s\r\n"
                "Connection: close\r\n",
                config->server_name);
                
        if (rio_unbuffered_write(client_fd, server_headers, strlen(server_headers)) == -1) {
            LOG_ERROR("Failed to write server headers to client");
            free(headers_section);
            free(cgi_output);
            return -1;
        }

        // Process and write CGI headers (skip Status header)
        char *header_line = strtok(headers_section, "\n");
        while (header_line) {
            // Remove \r if present
            char *cr = strchr(header_line, '\r');
            if (cr) *cr = '\0';
            
            // Skip Status header as we already processed it
            if (strncmp(header_line, "Status:", 7) != 0 && strlen(header_line) > 0) {
                if (rio_unbuffered_write(client_fd, header_line, strlen(header_line)) == -1 ||
                    rio_unbuffered_write(client_fd, "\r\n", 2) == -1) {
                    LOG_ERROR("Failed to write CGI header to client");
                    free(headers_section);
                    free(cgi_output);
                    return -1;
                }
            }
            
            header_line = strtok(NULL, "\n");
        }

        // Write header/body separator
        if (rio_unbuffered_write(client_fd, "\r\n", 2) == -1) {
            LOG_ERROR("Failed to write header separator to client");
            free(headers_section);
            free(cgi_output);
            return -1;
        }

        // Write body content
        size_t body_len = total_output - (body_section - cgi_output);
        if (body_len > 0) {
            if (rio_unbuffered_write(client_fd, body_section, body_len) == -1) {
                LOG_ERROR("Failed to write body to client");
                free(headers_section);
                free(cgi_output);
                return -1;
            }
        }

        // Cleanup
        free(headers_section);
        free(cgi_output);
        
        LOG_INFO("Successfully served dynamic content");
        return 0;
    }
}


char* generate_response_header(http_response* response) {
    if (!response) {
        LOG_ERROR("NULL response passed to generate_response_header");
        return NULL;
    }

    // Calculate approximate size needed for header buffer
    // Start with reasonable base size for status line and common headers
    size_t header_size = 256;
    
    // Add space for content-length digits (20 chars should be enough for even very large files)
    header_size += 20;
    
    // Add space for content-type
    if (response->content_type) 
        header_size += strlen(response->content_type) + 16; // "Content-Type: " + content
    
    // Add space for other headers if they exist
    if (response->date) 
        header_size += strlen(response->date) + HDR_DATE_PREFIX_LEN + CRLF_LEN;
        
    if (response->server) 
        header_size += strlen(response->server) + HDR_SERVER_PREFIX_LEN + CRLF_LEN;
        
    if (response->connection) 
        header_size += strlen(response->connection) + HDR_CONNECTION_PREFIX_LEN + CRLF_LEN;
        
    if (response->last_modified) 
        header_size += strlen(response->last_modified) + HDR_LASTMOD_PREFIX_LEN + CRLF_LEN;
        
    if (response->content_encoding) 
        header_size += strlen(response->content_encoding) + HDR_CONTENT_ENC_PREFIX_LEN + CRLF_LEN;
        
    if (response->cache_control) 
        header_size += strlen(response->cache_control) + HDR_CACHE_CTRL_PREFIX_LEN + CRLF_LEN;
        
    if (response->etag) 
        header_size += strlen(response->etag) + HDR_ETAG_PREFIX_LEN + CRLF_LEN;
    
    // Add space for extra headers
    for (int i = 0; i < response->extra_header_count; i++) {
        if (response->extra_header_names[i] && response->extra_header_values[i]) {
            header_size += strlen(response->extra_header_names[i]) + 
                          strlen(response->extra_header_values[i]) + 4; // name + ": " + value + "\r\n"
        }
    }
    
    // Allocate memory for the header
    char* header = (char*)malloc(header_size);
    if (!header) {
        LOG_ERROR("Failed to allocate memory for response header");
        return NULL;
    }
    
    // Start with empty string
    header[0] = '\0';
    
    // Add status line
    char status_line[128];
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
             response->status_code, response->reason ? response->reason : "Unknown");
    strcat(header, status_line);
    
    // Add standard headers if they exist
    if (response->date) {
        char date_header[128];
        snprintf(date_header, sizeof(date_header), "Date: %s\r\n", response->date);
        strcat(header, date_header);
    }
    
    if (response->server) {
        char server_header[128];
        snprintf(server_header, sizeof(server_header), "Server: %s\r\n", response->server);
        strcat(header, server_header);
    }
    
    if (response->connection) {
        char connection_header[128];
        snprintf(connection_header, sizeof(connection_header), "Connection: %s\r\n", response->connection);
        strcat(header, connection_header);
    }
    
    if (response->last_modified) {
        char last_modified_header[128];
        snprintf(last_modified_header, sizeof(last_modified_header), "Last-Modified: %s\r\n", 
                response->last_modified);
        strcat(header, last_modified_header);
    }
    
    // Add caching headers if they exist
    if (response->cache_control) {
        char cache_control_header[128];
        snprintf(cache_control_header, sizeof(cache_control_header), "Cache-Control: %s\r\n", 
                response->cache_control);
        strcat(header, cache_control_header);
    }
    
    if (response->etag) {
        char etag_header[128];
        snprintf(etag_header, sizeof(etag_header), "ETag: %s\r\n", response->etag);
        strcat(header, etag_header);
    }
    
    // Add content headers
    if (response->content_type) {
        char content_type_header[128];
        snprintf(content_type_header, sizeof(content_type_header), "Content-Type: %s\r\n", 
                response->content_type);
        strcat(header, content_type_header);
    }
    
    // Always include Content-Length
    char content_length_header[64];
    snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %zu\r\n", 
            response->content_length);
    strcat(header, content_length_header);
    
    if (response->content_encoding) {
        char content_encoding_header[128];
        snprintf(content_encoding_header, sizeof(content_encoding_header), "Content-Encoding: %s\r\n", 
                response->content_encoding);
        strcat(header, content_encoding_header);
    }
    
    // Add any extra headers
    for (int i = 0; i < response->extra_header_count; i++) {
        if (response->extra_header_names[i] && response->extra_header_values[i]) {
            char extra_header[256];
            snprintf(extra_header, sizeof(extra_header), "%s: %s\r\n", 
                    response->extra_header_names[i], response->extra_header_values[i]);
            strcat(header, extra_header);
        }
    }
    
    // Add the final CRLF that separates headers from body
    strcat(header, "\r\n");
    
    return header;
}

void destroy_response(http_response * response) {
    free(response->date);
    free(response->last_modified);
    for(int i = 0; i < response->extra_header_count; ++i) {
        free(response->extra_header_names[i]);
        free(response->extra_header_values[i]);
    }
    free(response->extra_header_names);
    free(response->extra_header_values);
}

