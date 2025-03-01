#include "../include/net.h"
#include <stdio.h>

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 3) {
        fprintf(stderr, "Please run as ./prog_name port_number IP");
    }
    char * port = argv[1];
    char * ip = argv[2];
    int clientfd = open_clientfd(ip, port);
    
    return 0;
}
