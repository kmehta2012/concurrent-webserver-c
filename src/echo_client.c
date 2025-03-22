#include "../include/net.h"
#include "../include/rio.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv)  {
    if(argc == 1 || argc > 3) {
        fprintf(stderr, "Please run as ./prog_name port_number IP");
        return -1;
    }
    char * port = argv[1];
    char * ip = argv[2];
    int clientfd = open_clientfd(ip, port);
    
    rio_buf stdin_buf; // buffer in which we'll store stdin contents to be writtent to clientfd
    char user_input[BUFFER_SIZE];
    char server_response[BUFFER_SIZE];
    rio_buf stdout_buf; // buffer in which we'll store server response to be written to stdout

    rio_init_buffer(STDIN_FILENO, &stdin_buf);
    rio_init_buffer(clientfd, &stdout_buf);

    /*
    1. Client reads from stdin and writes to its socket to send the read content to the server's connected descriptor
    2. Server echoes and sends it back to the client sockets
    3. Read from the client's socket and write to stdout
    4. Read the echo response from the server and print to stdout
    */

    printf("Enter line to echo : ");
    fflush(stdout);
    for(ssize_t total_bytes = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE); total_bytes != 0; total_bytes = rio_buffered_readline(&stdin_buf, user_input, BUFFER_SIZE)) {
        if(total_bytes == -1) {
            fprintf(stderr, "failed to read user input");
            return -1;
        }
        total_bytes = rio_unbuffered_write(clientfd, user_input, total_bytes); // write to the server
        if(total_bytes == -1) {
            fprintf(stderr, "failed to write user input to server");
            return -1;
        }
        
        total_bytes = rio_buffered_readline(&stdout_buf, server_response, total_bytes); // read response from server to a buffer
        if(total_bytes == -1) {
            fprintf(stderr, "failed to read echo response from server");
            return -1;
        }

        total_bytes = rio_unbuffered_write(STDOUT_FILENO, server_response, total_bytes); // write server response to STDOUT
        if(total_bytes == -1) {
            fprintf(stderr, "failed to write server response to stdout");
            return -1;
        }
        printf("Enter line to echo : ");
        fflush(stdout);
    }
    close(clientfd);
    return 0;
}
