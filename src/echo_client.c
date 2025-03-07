#include "../include/net.h"
#include "../include/rio.h"
#include <stdio.h>

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 3) {
        fprintf(stderr, "Please run as ./prog_name port_number IP");
    }
    char * port = argv[1];
    char * ip = argv[2];
    int clientfd = open_clientfd(ip, port);
    rio_buf stdin_buf;
    char user_input[BUFFER_SIZE];

    rio_init_buffer(0, &stdin_buf);

    for(ssize_t result = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE); result != 0; result = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE)) {
        if(result == -1) {
            fprintf(stderr, "failed to read user input");
            return -1;
        }
        rio_unbuffered_write(clientfd, &stdin_buf, BUFFER_SIZE); 
    }
    close(clientfd);
    return 0;
}
