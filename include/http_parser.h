// functions for parsing of http requests
#ifndef HTTP_PARSER
#define HTTP_PARSER
#include <stdbool.h>

#include "../include/config.h"
#include "../include/rio.h"

#define MAX_URI_LENGTH 4096

typedef enum {
    GET,
    POST,
    OPTIONS,
    HEAD,
    PUT,
    DELETE,
    TRACE
} HTTP_METHOD;

typedef enum {
    TEXT_HTML,
    TEXT_PLAIN,
    APPLICATION_POSTSCRIPT,
    IMAGE_GIF,
    IMAGE_PNG,
    IMAGE_JPEG
}MIME_TYPE;

typedef enum {
    HTTP_1_0,
    HTTP_1_1
} HTTP_VERSION;

typedef struct {
    HTTP_VERSION version;
    MIME_TYPE mime_type;
    HTTP_METHOD method;
    char* path;           // The absolute file path
    bool is_dynamic;      // Flag indicating if this is a dynamic request
    char** param_names;   // Array of parameter names (if dynamic)
    char** param_values;  // Array of parameter values (if dynamic)
    int param_count;      // Number of parameters
}http_request;

/*
parses request line -  METHOD URI HTTP_VERSION. Assumes that rio_buf * request adheres to the syntax and throws an exception otherwise

Args
    rio_buf * request : buffer in which the http request is stored
Returns
    
*/
http_request * parse_http_request(char * client_request, http_request * request, server_config * config);


int parse_request_line(char * request_line, http_request * request);

/**
 * Parses URI to fill the following fields of request:
 * 1. path: relative path of the requested file (wrt to the server document root)
 * 2. mime_type: MIME_TYPE determines if file is dynamic or static
 * 3. is_dynamic: flag indicating if this is a dynamic request
 * 4. param_names: array of parameter names (if dynamic)
 * 5. param_values: array of parameter values (if dynamic)
 * 6. param_count: number of parameters
 * 
 * Args:
 *    char *URI: URI from the HTTP request
 *    http_request *request: request structure to fill
 *    server_config *config: server configuration settings
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int parse_uri(char * URI, http_request * request, server_config * config);


int parse_request_headers(rio_buf * request);

int url_decode(char * str);

/*
Returns the MIME type of a file with the passed path
*/
MIME_TYPE get_mime_type(const char *path);

/**
 * @brief 
 * Free's all dynamically allocated members of request : 
 * 1. path
 * 2. param_names
 * 3. param_values
 * 
 * It is assumed that request itself is statically allocated by the caller or if dynamically 
 * allocated - it is free'd by the called
 * 
 * @param 
 * request : http_request whose resources are to be deallocated
 */
void destroy_request(http_request * request);

/**
 * @brief 
 * Initializes the members of a http request struct to default values
 * 
 * @param 
 * request : http request to initialize. 
 */
void initialize_request(http_request * request);



#endif