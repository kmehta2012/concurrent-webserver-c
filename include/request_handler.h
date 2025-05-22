// header to carry out http requests
#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#include "http_parser.h"
#include "config.h"

// Header field name lengths (including colon and space)
#define HDR_DATE_PREFIX_LEN         6   // "Date: "
#define HDR_SERVER_PREFIX_LEN       8   // "Server: "
#define HDR_CONNECTION_PREFIX_LEN   11  // "Connection: "
#define HDR_LASTMOD_PREFIX_LEN      14  // "Last-Modified: "
#define HDR_CONTENT_ENC_PREFIX_LEN  17  // "Content-Encoding: "
#define HDR_CACHE_CTRL_PREFIX_LEN   13  // "Cache-Control: "
#define HDR_ETAG_PREFIX_LEN         6   // "ETag: "
#define HDR_CONTENT_TYPE_PREFIX_LEN 14  // "Content-Type: "
#define HDR_CONTENT_LEN_PREFIX_LEN  16  // "Content-Length: "

// Each header field also needs 2 bytes for CRLF
#define CRLF_LEN                    2   // "\r\n"

// Structure to hold HTTP response details
typedef struct {
    // Status information
    int status_code;         // HTTP status code (200, 404, etc.)
    char *reason;            // Status reason phrase (OK, Not Found, etc.)
    
    // Standard required headers
    char *server;            // Server identification
    char *date;              // Response generation timestamp. Will be dynamically allocated. Must be freed
    
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
 * Initializes an HTTP response structure with default values
 * 
 * Args:
 *    http_response *response: Pointer to response structure to initialize
 */
void initialize_response(http_response *response);




/**
 * Sets content-related headers in the HTTP response based on the file
 * 
 * Args:
 *    int fd: Open file descriptor for the file being served
 *    http_request *request: The parsed HTTP request
 *    http_response *response: Response structure to update
 *    const char *file_path: Path to the file (for logging)
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int set_content_headers(int fd, http_request *request, http_response *response, const char *file_path);

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
int serve_dynamic(http_request *request, http_response * response, int client_fd, server_config *config);

/**
 * Returns response header for client (including response line). Follows the following order for the headers. returned pointer must be free'd by the caller
 * 
 * A common ordering pattern is:
 * Status line
 * Date
 * Server
 * Connection
 * Cache-related headers (Cache-Control, Last-Modified, ETag)
 * Content-related headers (Content-Type, Content-Length, Content-Encoding)
 * Custom headers
 * 
 * Args:
 *    int client_fd: Client connection file descriptor
 *    int status_code: HTTP status code
 *    const char *reason: Status reason phrase
 * 
 * Returns:
 *    header on success, NULL on error
 */
char * generate_response_header(http_response * response);

/**
 * @brief Get the absolute path of requested file.
 * Concatenates request->path with config->document_root
 * 
 * The caller must free the returned string. 
 * 
 * @param request : client request struct containing the relative path of the file
 * @param config : server config struct containing the absolute path 
 * @return The absolute file path on success, NULL if length of absolute path > PATH_MAX - 1
 */
char * get_absolute_path(http_request * request, server_config * config);



void initialize_response(http_response * response);

void destroy_response(http_response * response);
#endif