#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Helper function to trim whitespace from a string
 */
static void trim(char *str) {
    if (!str) return;
    
    // Trim leading space
    char *start = str;
    while (*start && isspace(*start)) start++;
    
    // If all spaces, return empty string
    if (!*start) {
        *str = '\0';
        return;
    }
    
    // Trim trailing space
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    // Write new null terminator
    *(end + 1) = '\0';
    
    // If there was leading space, shift string
    if (start != str) {
        memmove(str, start, (end - start) + 2);
    }
}

/**
 * Helper function to duplicate string with null check
 */
static char* safe_strdup(const char *str) {
    if (!str) return NULL;
    return strdup(str);
}

/**
 * Initialize configuration with default values
 */
void config_init(server_config *config) {
    if (!config) {
        LOG_ERROR("NULL config pointer passed to config_init");
        return;
    }
    
    // Set default values
    config->port = safe_strdup("8080");
    config->document_root = safe_strdup("./public/");
    config->cgi_bin_path = safe_strdup("./public/cgi-bin/");
    config->server_name = safe_strdup("TuringBolt/0.1");
    config->log_directory = safe_strdup("./logs/");
    config->dynamic_dir_name = safe_strdup("cgi-bin");
    config->static_dir_name = safe_strdup("static");
    config->thread_pool_size = 4;
    config->connection_timeout = 60;
    config->enable_logging = true;
    
    LOG_INFO("Configuration initialized with default values");
}

/**
 * Load configuration from file
 */
bool config_load(server_config *config, const char *filename) {
    if (!config || !filename) {
        LOG_ERROR("NULL parameter passed to config_load");
        return false;
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        LOG_ERROR("Failed to open config file: %s", filename);
        return false;
    }
    
    LOG_INFO("Loading configuration from %s", filename);
    
    char line[512];
    char current_section[64] = "";
    
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[len-1] = '\0';
        
        // Skip comments and empty lines
        if (line[0] == ';' || line[0] == '\0' || isspace(line[0]))
            continue;
        
        // Check for section header
        if (line[0] == '[') {
            char *end = strchr(line, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, line + 1, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0';
                LOG_DEBUG("Parsing section: %s", current_section);
            }
            continue;
        }
        
        // Parse key-value pair
        char *value = strchr(line, '=');
        if (!value)
            continue;
            
        *value = '\0';
        value++;
        
        // Trim whitespace
        char *key = line;
        trim(key);
        trim(value);
        
        LOG_DEBUG("Parsed key-value pair: %s = %s", key, value);
        
        // Set config values based on section and key
        if (strcmp(current_section, "Server") == 0) {
            if (strcmp(key, "Port") == 0) {
                free(config->port);
                config->port = safe_strdup(value);
            }
            else if (strcmp(key, "DocumentRoot") == 0) {
                free(config->document_root);
                config->document_root = safe_strdup(value);
            }
            else if (strcmp(key, "ServerName") == 0) {
                free(config->server_name);
                config->server_name = safe_strdup(value);
            }
        }
        else if (strcmp(current_section, "Directories") == 0) {
            if (strcmp(key, "CgiBinPath") == 0) {
                free(config->cgi_bin_path);
                config->cgi_bin_path = safe_strdup(value);
            }
            else if (strcmp(key, "DynamicDirName") == 0) {
                free(config->dynamic_dir_name);
                config->dynamic_dir_name = safe_strdup(value);
            }
            else if (strcmp(key, "StaticDirName") == 0) {
                free(config->static_dir_name);
                config->static_dir_name = safe_strdup(value);
            }
        }
        else if (strcmp(current_section, "Performance") == 0) {
            if (strcmp(key, "ThreadPoolSize") == 0) {
                int thread_pool_size = atoi(value);
                if (thread_pool_size > 0) {
                    config->thread_pool_size = (unsigned int)thread_pool_size;
                } else {
                    LOG_WARN("Invalid ThreadPoolSize value: %s, using default", value);
                }
            }
            else if (strcmp(key, "ConnectionTimeout") == 0) {
                int timeout = atoi(value);
                if (timeout > 0) {
                    config->connection_timeout = (unsigned int)timeout;
                } else {
                    LOG_WARN("Invalid ConnectionTimeout value: %s, using default", value);
                }
            }
        }
        else if (strcmp(current_section, "Logging") == 0) {
            if (strcmp(key, "EnableLogging") == 0) {
                if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
                    config->enable_logging = true;
                } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
                    config->enable_logging = false;
                } else {
                    LOG_WARN("Invalid EnableLogging value: %s, using default", value);
                }
            }
            else if (strcmp(key, "LogDirectory") == 0) {
                free(config->log_directory);
                config->log_directory = safe_strdup(value);
            }
        }
        // Unknown section or key - ignore with warning
        else {
            LOG_WARN("Unknown configuration section: %s", current_section);
        }
    }
    
    fclose(file);
    
    // Validate essential configuration
    bool valid = true;
    if (!config->port) {
        LOG_ERROR("Required configuration Port is missing");
        valid = false;
    }
    if (!config->document_root) {
        LOG_ERROR("Required configuration DocumentRoot is missing");
        valid = false;
    }
    
    // Ensure directories end with '/'
    if (config->document_root) {
        size_t len = strlen(config->document_root);
        if (len > 0 && config->document_root[len-1] != '/') {
            char *new_root = malloc(len + 2);
            if (new_root) {
                strcpy(new_root, config->document_root);
                new_root[len] = '/';
                new_root[len+1] = '\0';
                free(config->document_root);
                config->document_root = new_root;
                LOG_INFO("Added trailing slash to DocumentRoot");
            }
        }
    }
    
    if (config->log_directory) {
        size_t len = strlen(config->log_directory);
        if (len > 0 && config->log_directory[len-1] != '/') {
            char *new_dir = malloc(len + 2);
            if (new_dir) {
                strcpy(new_dir, config->log_directory);
                new_dir[len] = '/';
                new_dir[len+1] = '\0';
                free(config->log_directory);
                config->log_directory = new_dir;
                LOG_INFO("Added trailing slash to LogDirectory");
            }
        }
    }
    
    if (valid) {
        LOG_INFO("Configuration loaded successfully");
    } else {
        LOG_ERROR("Configuration loading failed due to missing required values");
    }
    
    return valid;
}

/**
 * Free allocated resources
 */
void config_cleanup(server_config *config) {
    if (!config) return;
    
    free(config->port);
    free(config->document_root);
    free(config->cgi_bin_path);
    free(config->server_name);
    free(config->log_directory);
    free(config->dynamic_dir_name);
    free(config->static_dir_name);
    
    // Reset values to prevent use-after-free
    config->port = NULL;
    config->document_root = NULL;
    config->cgi_bin_path = NULL;
    config->server_name = NULL;
    config->log_directory = NULL;
    config->dynamic_dir_name = NULL;
    config->static_dir_name = NULL;
    
    LOG_INFO("Configuration resources cleaned up");
}