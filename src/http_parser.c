#include "http_parser.h"
#include <string.h>
#include <strings.h>
#include "rio.h"
#include "request_handler.h"
#include "utils.h"
#include <stdio.h>
#include "logger.h"
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include "logger.h"

#define MAX_URI_LENGTH 4096


/*
TO DO:

Implement sanitization and normalization to avoid injections. 
Will do this probably after STAGE 3
*/


http_request * parse_http_request(char * client_request, http_request * request, server_config * config) {
    if (!request || !client_request || !config) {
        LOG_ERROR("NULL parameter passed to parse_http_request");
        return NULL;
    }
    
    // 1. Split the stirng by \r\n pair and strip \r\n from each splitted string 

    // 2. Read the HTTP method and the URI, call the respective method from request_handler.c to carry out the method on the file given by the URI
    char * crlf = strstr(client_request, "\r\n");
    if (!crlf) {
        LOG_ERROR("Malformed request - no CRLF found");
        return NULL;
    }
    int line_length = crlf - client_request; 
    if(line_length >= BUFFER_SIZE) { // We need it to be atleast one smaller than buffer size to incorporate a null character
        LOG_ERROR("Request line length exceeds maximum allowed length");
        return NULL;
    }
    char request_line[BUFFER_SIZE];
    strncpy(request_line, client_request, line_length);
    request_line[line_length] = '\0';
    
    char METHOD[BUFFER_SIZE], URI[BUFFER_SIZE], VERSION[BUFFER_SIZE]; 
    char sscanf_format_string[100];
    sprintf(sscanf_format_string, "%%%ds %%%ds %%%ds",
        BUFFER_SIZE - 1,
        BUFFER_SIZE - 1,
        BUFFER_SIZE - 1);

    int result = sscanf(request_line, sscanf_format_string, METHOD, URI, VERSION);
    
    if(strlen(URI) > MAX_URI_LENGTH) {
        LOG_ERROR("URI path exceeds maximum allowed length");
        return NULL;
    }
    // Validate parsing success
    if (result != 3) {
    // Handle error - malformed request
        LOG_ERROR("Error parsing request line");
        return NULL;
    }

    
    // Will add support for other methods soon
    if(strcmp("GET", METHOD)) {
        LOG_ERROR("Server only supports GET requests. Current HTTP request : %s", METHOD);
        return NULL;
    }
    if(strcmp("HTTP/1.1", VERSION) != 0 && strcmp("HTTP/1.0", VERSION) != 0){
        LOG_ERROR("Only supports HTTP_1_1 and HTTP_1_0. Requested HTTP Version : %s. NOT SUPPORTED", VERSION);
        return NULL;
    }
    if(URI[0] != '/'){
        LOG_ERROR("Invalid URI format. URI must start with /. Passed URI starts with : %c", URI[0]);
        return NULL;
    }
    // Add a check to ensure that URI fits in BUFFER_SIZE

    request->version = (strcmp(VERSION, "HTTP/1.0") == 0) ? HTTP_1_0 : HTTP_1_1;
    LOG_DEBUG("Set version to %s", VERSION);
    request->method = GET;
    LOG_DEBUG("Set method to %s", METHOD);
    
    if(parse_uri(URI, request, config) == -1) {
        LOG_ERROR("Parsing failed - Failed to parse URI");
        return NULL;
    }

    return request;
}


int parse_uri(char *URI, http_request *request, server_config *config) {
    
    if(url_decode(URI)) {
        LOG_ERROR("Failed to decode URI encoding.");
        return -1;
    }

    char *query_string = NULL;
    char * uri_ptr = URI;

    // Check if URI contains query string. 
    // strchr(src, target) returns pointer to the first occurrence of char target in string src or NULL if nothing is found 
    query_string = strchr(URI, '?');
    if (query_string) {
        *query_string = '\0';  // Split URI at '?'
        query_string++;        // Move pointer to start of query string
    }
    
    // Determine if request is for static or dynamic content. Potential problem wouldn't uri_copy start with the backslash
    request->is_dynamic = false;
    uri_ptr += 1; // to skip / so that uri_ptr points to the static/dynamic directory or just '\0'
    size_t dynamic_dir_len = strlen(config->dynamic_dir_name);
    if (!strncmp(uri_ptr, config->dynamic_dir_name, dynamic_dir_len)) {
        // Check if this is a complete path component by verifying that
        // the next character is either '/' or '\0'
        if (uri_ptr[dynamic_dir_len] == '/' || uri_ptr[dynamic_dir_len] == '\0') {
            request->is_dynamic = true;
        }
    }
    
    
    // Set path in request. 
    // The strdup() function allocates sufficient memory for a copy of the string s1, does the copy, and returns a pointer to it.  The pointer may subsequently be used as an argument to the function free(3).
    request->path = strdup(URI); 
    if (!request->path) {
        LOG_ERROR("Memory allocation failed for request path. ERROR : %s", strerror(errno));
        return -1;
    }
    // Determine MIME type based on file extension
    request->mime_type = get_mime_type(request->path);
    
    // Process query string for dynamic requests
    if (request->is_dynamic && query_string) {
        // Count parameters
        int count = 1;  // Start with 1 for first parameter
        for (char *c = query_string; *c; c++) {
            if (*c == '&') {
                count++;
            }
        }
        
        // Allocate memory for parameter arrays
        request->param_count = count;
        request->param_names = (char**)malloc(count * sizeof(char*));
        request->param_values = (char**)malloc(count * sizeof(char*));
        
        // free(NULL) is fine and does nothing
        if (!request->param_names || !request->param_values) {
            LOG_ERROR("Memory allocation failed for parameters");
            free(request->param_names);
            free(request->param_values);
            request->param_names = NULL;
            request->param_values = NULL;
            request->param_count = 0;
            return -1;
        }
        
        // Initialize arrays to NULL for proper cleanup in case of error
        for (int i = 0; i < count; i++) {
            request->param_names[i] = NULL;
            request->param_values[i] = NULL;
        }
        // strtok() modifies the query_string in-place by inserting null terminators in-place of the passed delimeter. 
        char *token = strtok(query_string, "&");
        int param_index = 0;
        
        while (token && param_index < count) {
            char *value = strchr(token, '=');
            if (value) {
                *value = '\0';  // Split token at '='
                value++;        // Move to value portion
                request->param_names[param_index] = strdup(token);
                request->param_values[param_index] = strdup(value);
                
                if (!request->param_names[param_index] || !request->param_values[param_index]) {
                    LOG_ERROR("Memory allocation failed for parameter strings");
                    // Cleanup will be handled by caller
                    return -1;
                }
            } else {
                // Handle parameters without values (e.g., "flag" in "?flag")
                request->param_names[param_index] = strdup(token);
                request->param_values[param_index] = strdup("");  // Empty string for value
                
                if (!request->param_names[param_index] || !request->param_values[param_index]) {
                    LOG_ERROR("Memory allocation failed for parameter strings");
                    return -1;
                }
            }
            
            param_index++;
            token = strtok(NULL, "&");
        }
    } else {
        // Non-dynamic requests don't have parameters
        request->param_count = 0;
        request->param_names = NULL;
        request->param_values = NULL;
    }
    
    return 0;
}

/**
 * Decodes URL-encoded string in-place.
 * Converts %XX hex sequences to their character equivalents.
 * 
 * Args:
 *    char *str: URL-encoded string to decode
 * 
 * Returns:
 *    0 on success, -1 on error
 */
int url_decode(char *str) {
    if (!str) return -1;
    
    char *dest = str;
    char *src = str;
    
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            // Try to convert hex sequence
            char hex[3] = {src[1], src[2], 0};
            char *end;
            long value = strtol(hex, &end, 16);
            
            if (end != hex + 2) {
                // Not a valid hex sequence, copy as is
                *dest++ = *src++;
            } else {
                *dest++ = (char)value;
                src += 3;
            }
        } else if (*src == '+') {
            // Convert + to space
            *dest++ = ' ';
            src++;
        } else {
            // Copy character as is
            *dest++ = *src++;
        }
    }
    *dest = '\0';  // Null terminate
    
    return 0;
}

/**
 * Determines MIME type based on file extension.
 * 
 * Args:
 *    const char *path: File path to check
 * 
 * Returns:
 *    MIME_TYPE enum value
 */
MIME_TYPE get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    
    if (!ext) {
        return TEXT_PLAIN;  // Default to plain text if no extension
    }
    
    ext++;  // Move past the dot
    
    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) {
        return TEXT_HTML;
    } else if (strcasecmp(ext, "txt") == 0) {
        return TEXT_PLAIN;
    } else if (strcasecmp(ext, "ps") == 0) {
        return APPLICATION_POSTSCRIPT;
    } else if (strcasecmp(ext, "gif") == 0) {
        return IMAGE_GIF;
    } else if (strcasecmp(ext, "png") == 0) {
        return IMAGE_PNG;
    } else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
        return IMAGE_JPEG;
    } else {
        return TEXT_PLAIN;  // Default to plain text for unknown types
    }
}

void destroy_request(http_request * request) {
    if(request) {
        free(request->path); // maintain the invariant that either this has not been free'd or is NULL
        LOG_DEBUG("Free'd request->path");
        if(request->param_count > 0) {
            for(int i = 0; i < request->param_count; ++i) {
                free(request->param_names[i]);
                free(request->param_values[i]);
            }
            
            free(request->param_names);
            LOG_DEBUG("Free'd request->param_names");
            free(request->param_values);
            LOG_DEBUG("Free'd request->param_values");
        }
    }
}

void initialize_request(http_request *request) {
    // Set pointers to NULL
    request->path = NULL;
    request->param_names = NULL;
    request->param_values = NULL;
    
    // Set integer values to 0
    request->param_count = 0;
    
    // Set enum values to their default/initial states
    request->method = GET;          // Default to GET as the most common method
    request->version = HTTP_1_1;    // Default to HTTP/1.1 as the most common version
    request->mime_type = TEXT_PLAIN; // Default to plain text
    
    // Set boolean values to false
    request->is_dynamic = false;    // Default to static content
}

