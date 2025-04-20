#include "../include/http_parser.h"
#include <string.h>
#include <strings.h>
#include "../include/rio.h"
#include "../include/request_handler.h"
#include <stdio.h>
#include <logger.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include "../include/logger.h"



http_request * parse_http_request(rio_buf * client_request, http_request * request, server_config * config) {
    // 1. Split the stirng by \r\n pair and strip \r\n from each splitted string 

    // 2. Read the HTTP method and the URI, call the respective method from request_handler.c to carry out the method on the file given by the URI
    char request_line[BUFFER_SIZE];
    rio_buffered_readline(client_request, request_line, BUFFER_SIZE);
    char METHOD[BUFFER_SIZE], URI[BUFFER_SIZE], VERSION[BUFFER_SIZE]; 
    
    // NEED TO MAKE THIS SAFE. 
    sscanf(request_line, "%s %s %s", METHOD, URI, VERSION);

    
    // Will add support for other methods soon
    if(!strcmp("GET", METHOD)) {
        LOG_ERROR("Server only supports GET requests. Current HTTP request : %s", METHOD);
        return -1;
    }
    if(!strcmp("HTTP/1.1", VERSION) || !strcmp("HTTP/1.0", VERSION)){
        LOG_ERROR("Only supports HTTP_1_1 and HTTP_1_0. Requested HTTP Version : %s. NOT SUPPORTED", VERSION);
        return -1;
    }
    if(URI[0] != '\\'){
        LOG_ERROR("Invalid URI format. URI must start with \\. Passed URI starts with : %c", URI[0]);
        return -1;
    }
    // Add a check to ensure that URI fits in BUFFER_SIZE

    request->version = VERSION;
    LOG_DEBUG("Set version to %s", VERSION);
    request->method = GET;
    LOG_DEBUG("Set method to %s", METHOD);
    
    if(parse_uri(URI, request, config) == -1) {
        LOG_DEBUG("Parsing failed - Failed to parse URI");
        return -1;
    }

    return request;
}


int parse_uri(char *URI, http_request *request, server_config *config) {
    char *query_string = NULL;
    char uri_copy[BUFFER_SIZE];
    
    // Make a copy of URI for modification. We assert that URI is of size atmost BUFFER_SIZE-1 in the caller code
    strncpy(uri_copy, URI, BUFFER_SIZE-1);
    uri_copy[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
    char * uri_ptr = uri_copy;
    
    // Check if URI contains query string. 
    // strchr(src, target) returns pointer to the first occurrence of char target in string src or NULL if nothing is found 
    query_string = strchr(uri_copy, '?');
    if (query_string) {
        *query_string = '\0';  // Split URI at '?'
        query_string++;        // Move pointer to start of query string
    }
    
    // Determine if request is for static or dynamic content. Potential problem wouldn't uri_copy start with the backslash
    request->is_dynamic = false;
    uri_ptr += 1;
    if(strncmp(uri_ptr, config->dynamic_dir_name, strlen(config->dynamic_dir_name)) == 0) {
        request->is_dynamic = true;
    }
    
    // Decode URL encoding
    url_decode(uri_copy);
    
    // Set path in request
    request->path = strdup(uri_copy);
    if (!request->path) {
        LOG_ERROR("Memory allocation failed for request path. ERROR : %s", strerror(errno));
        return -1;
    }
    
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
        
        // Parse parameters
        char query_copy[BUFFER_SIZE];
        strncpy(query_copy, query_string, BUFFER_SIZE);
        query_copy[BUFFER_SIZE - 1] = '\0';
        
        char *token = strtok(query_copy, "&");
        int param_index = 0;
        
        while (token && param_index < count) {
            char *value = strchr(token, '=');
            if (value) {
                *value = '\0';  // Split token at '='
                value++;        // Move to value portion
                
                // Decode URL encoding for both name and value
                url_decode(token);
                url_decode(value);
                
                request->param_names[param_index] = strdup(token);
                request->param_values[param_index] = strdup(value);
                
                if (!request->param_names[param_index] || !request->param_values[param_index]) {
                    LOG_ERROR("Memory allocation failed for parameter strings");
                    // Cleanup will be handled by caller
                    return -1;
                }
            } else {
                // Handle parameters without values (e.g., "flag" in "?flag")
                url_decode(token);
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
    
    // Determine MIME type based on file extension
    request->mime_type = get_mime_type(uri_copy);
    
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

