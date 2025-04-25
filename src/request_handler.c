#include "../include/request_handler.h"
#include "../include/rio.h"

int execute_request(http_request *request, int client_fd, server_config *config) {

    if(request->is_dynamic) {
        return serve_dynamic(request, client_fd, config);
    }
    else {
        return serve_static(request, client_fd, config);
    }

}

int serve_static(http_request *request, int client_fd, server_config *config) {
    /*
    1. Check if request->path actually exists, if not send back the correct response code 
    2. If it does then rio_unbuffered_write the file to client_fd
    

    */



}