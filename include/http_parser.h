// functions for parsing of http requests
#ifndef HTTP_PARSER
#define HTTP_PARSER
#include <stdbool.h>

#include "../include/config.h"
#include "../include/rio.h"




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
    char* path;           // The relative file path
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
http_request * parse_http_request(rio_buf * client_request, http_request * request, server_config * config);


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

MIME_TYPE get_mime_type(const char *path);

#endif