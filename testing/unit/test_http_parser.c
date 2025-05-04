#include <http_parser.h>
#include <check.h>
#include "rio.h"
#include "logger.h"
#include "config.h"


static int total_tests = 0;
static int passed_tests = 0;


int main() {
    total_tests += 1;
    http_request request;
    rio_buf client_request = rio_init_buffer()
}



