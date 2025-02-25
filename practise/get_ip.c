// Implementation of nslookup - only the IPV4 addresses
#include <stdio.h> // standard IO
#include <sys/types.h> // idk
#include <sys/socket.h> // socket functions
#include <netdb.h> // getaddrinfo
#include <stdlib.h> // dynamic memory 
#include <arpa/inet.h>
#include <string.h>

typedef struct addrinfo addrinfo;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

void walk_addrinfo_list(addrinfo ** results) {
    addrinfo * node = *results;
    int count = 0;
    
    while(node) {
        ++count;
        sockaddr_in * sock_addr = (sockaddr_in *) node->ai_addr;
        char dotted_decimal_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sock_addr->sin_addr), dotted_decimal_ip, INET_ADDRSTRLEN); // using getnameinfo() would be easier
        printf("IP-Address %d : %s\n", count, dotted_decimal_ip);
        node = node->ai_next;
    }
}

int main(int argc, char ** argv) {
    if(argc <= 1 || argc > 2) {
        printf("Please specify exactly one domain name. Current domain name count : %d \n", argc - 1);
        return -1;
    }

    char * domain_name = argv[1];
    addrinfo * hints = malloc(sizeof(addrinfo)); 
    addrinfo *result = NULL;
    
    memset(hints, 0, sizeof(addrinfo));

    hints->ai_socktype = SOCK_STREAM; // only need TCP socket address structure per ip-adreess
    hints->ai_family = AF_INET; // only need IPV4 addresses

    int success_status = getaddrinfo(domain_name, NULL, hints, &result);
    if(success_status) {
        printf("Error: %s",  gai_strerror(success_status));
        return -1;
    }

    walk_addrinfo_list(&result);
    freeaddrinfo(result);
    free(hints);
    return 0;
    
}
