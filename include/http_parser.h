// functions for parsing of http requests
#ifndef HTTP_PARSER
#define HTTP_PARSER
#include <stdbool.h>

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
int parse_http_request(rio_buf * request);


int parse_request_line(char * request_line, http_request * request);

/*
Parses URI to fill the following fields of request:
1. path : relative path of the requested file (wrt to the server document root), 
2. mime_type : MIME_TYPE determines if file is dynamic or static, 
3. path : and gets the arguments if the file is dynamic. 
4. is_dynamic
5. param_names
6. param_values
7. param_count
Args
    URI : URI of a HTTP request
*/
int parse_uri(char * URI, http_request * request);


int parse_request_headers(rio_buf * request);

#endif