// header to carry out http requests
#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#include "http_parser.h"
#include "config.h"

// Structure to hold HTTP response details
typedef struct {
    // Status information
    int status_code;         // HTTP status code (200, 404, etc.)
    char *reason;            // Status reason phrase (OK, Not Found, etc.)
    
    // Standard required headers
    char *server;            // Server identification
    char *date;              // Response generation timestamp
    
    // Content-related headers
    char *content_type;      // MIME type of the content
    size_t content_length;   // Length of body in bytes
    char *content_encoding;  // Optional encoding (gzip, etc.)
    char *last_modified;     // When the resource was last changed
    
    // Connection management
    char *connection;        // Connection control (close, keep-alive)
    
    // Caching control
    char *cache_control;     // Caching directives
    char *etag;              // Entity tag for validation
    
    // Body content
    char *body;              // Response body (or file path if is_file=true)
    bool is_file;            // True if body is a file path
    
    // Extra headers
    char **extra_header_names;   // Array of extra header names
    char **extra_header_values;  // Array of extra header values
    int extra_header_count;      // Count of extra headers
} http_response;

/**
 * Executes an HTTP request and sends the appropriate response
 * 
 * Args:
 *    http_request *request: Parsed HTTP request
 *    int client_fd: Client connection file descriptor
 *    server_config *config: Server configuration
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int execute_request(http_request *request, int client_fd, server_config *config);

/**
 * Serves static content and sends response to client
 * 
 * Args:
 *    http_request *request: Parsed HTTP request
 *    int client_fd: Client connection file descriptor
 *    server_config *config: Server configuration
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int serve_static(http_request *request, http_response * response, int client_fd, server_config *config);

/**
 * Serves dynamic content by executing the CGI script
 * 
 * Args:
 *    http_request *request: Parsed HTTP request
 *    int client_fd: Client connection file descriptor
 *    server_config *config: Server configuration
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int serve_dynamic(http_request *request, int client_fd, server_config *config);

/**
 * Sends an HTTP error response to the client
 * 
 * Args:
 *    int client_fd: Client connection file descriptor
 *    int status_code: HTTP status code
 *    const char *reason: Status reason phrase
 * 
 * Returns:
 *    0 on success, -1 on error
 */


char * generate_response_header(http_response * response);

void initialize_response(http_response * response);

#endif