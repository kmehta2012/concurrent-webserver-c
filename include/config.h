#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>


typedef struct {
    char *port;                // Port to listen on
    char *document_root;       // Path to web document root directory. Must end with a forward slash 
    char *cgi_bin_path;        // Path to CGI scripts directory
    char * server_name;        // Official name of the server
    bool enable_logging;       // Whether to enable logging
    char *log_directory;       // Directory for log files
    char * dynamic_dir_name; // Name of directory containing dynamic content
    char * static_dir_name; // Name of directory containing static contant
    unsigned int thread_pool_size;  // Number of worker threads (for threaded version)
    unsigned int connection_timeout; // Connection timeout in seconds
    // Other configuration parameters
} server_config;

// Initialize configuration with default values
void config_init(server_config *config);

// Load configuration from file
bool config_load(server_config *config, const char *filename);

// Free allocated resources
void config_cleanup(server_config *config);

#endif