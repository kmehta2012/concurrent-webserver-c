#include "../include/net.h"
#include "../include/rio.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 3) {
        fprintf(stderr, "Please run as ./prog_name port_number IP");
    }
    char * port = argv[1];
    char * ip = argv[2];
    int clientfd = open_clientfd(ip, port);
    rio_buf stdin_buf;
    char user_input[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    rio_buf stdout_buf;

    rio_init_buffer(STDIN_FILENO, &stdin_buf);
    rio_init_buffer(clientfd, &stdout_buf);

    /*
    1. Client reads from stdin and writes to its socket to send the read content to the server's connected descriptor
    2. Server echoes and sends it back to the client sockets
    3. Read from the client's socket and write to stdout
    4. Read the echo response from the server and prirnt to stdout
    */

    for(ssize_t result = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE); result != 0; result = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE)) {
        if(result == -1) {
            fprintf(stderr, "failed to read user input");
            return -1;
        }
        result = rio_unbuffered_write(clientfd, user_input, BUFFER_SIZE); // write to the server
        if(result == -1) {
            fprintf(stderr, "failed to write user input to server");
            return -1;
        }
        
        result = rio_buffered_readline(&stdout_buf, server_response, BUFFER_SIZE); // read response from server to a buffer
        if(result == -1) {
            fprintf(stderr, "failed to read echo response from server");
            return -1;
        }

        result = rio_unbuffered_write(STDOUT_FILENO, &stdout_buf, BUFFER_SIZE); // write server response to STDOUT
        if(result == -1) {
            fprintf(stderr, "failed to write server response to stdout");
            return -1;
        }
    }
    close(clientfd);
    return 0;
}
