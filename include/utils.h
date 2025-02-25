#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

enum process_type {
    CLIENT,
    SERVER
};

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct addrinfo addrinfo;
typedef enum process_type process_type;

/*
Establishes a TCP connection with a server running on host hostname and listening on port number
Supports IPV4 addresses only 

Args 
    char * hostname : IPV4 address or the domain name associated with the server
    char * port : port number associated with the server
Returns
    0 on sucess and -1 on failure
*/
int open_clientfd(char * hostname, char * port);


/*
Returns a listening descriptor that is ready to receive connection requests on port port. 

Args
    char * port : port number (NOT service name) on which the web server will run
Returns
    The descriptor of the listening socket on success. -1 on failure
*/
int open_listenfd(char * port);


#endif