#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <string.h>


/*
DEBUG Level
    Use DEBUG logging when you need to track detailed program flow that would be useful during development and troubleshooting. 
    This level is for information that would be too verbose for normal operation.

    DEBUG logs help you trace the execution path when hunting for hard-to-find bugs. 
    They're typically disabled in production environments to avoid overwhelming your logs with too much information.

INFO level
    INFO logs record normal application behavior and milestones. 
    They confirm that things are working as expected and provide a high-level narrative of what your application is doing.

    INFO logs should give you enough information to understand what your application is doing without overwhelming you with details. 
    They're often kept enabled in production for basic monitoring and troubleshooting.

WARN level
    WARN logs indicate potential problems that haven't caused a failure yet but might need attention. 
    They signal that something unexpected happened, but the application can still function.

    WARN logs help you identify issues before they become critical errors. 
    They should always be enabled in production environments so you can proactively address potential problems.

ERROR level
    ERROR logs indicate that something has failed and the application couldn't complete an operation. These represent actual problems that need attention.

    ERROR logs represent actual failures that might affect functionality and should trigger alerts in production environments. 
    They provide essential information for diagnosing and fixing problems.



*/
typedef enum {
    LOG_DEBUG,
    LOG_INFO,  
    LOG_WARN,
    LOG_ERROR
} log_level_t;

static const char* level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

#define LOG(level, fmt, ...) do { \
    time_t now = time(NULL); \
    char time_str[20]; \
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
    fprintf(stderr, "[%s] [%s] " fmt "\n", time_str, level_names[level], ##__VA_ARGS__); \
} while(0)

#define LOG_DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif