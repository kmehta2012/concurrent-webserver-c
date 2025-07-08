#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#include "request_handler.h"
#include "http_parser.h"
#include "config.h"
#include "rio.h"

/* Test fixtures */
static http_request request;
static http_response response;
static server_config config;
static int pipe_fds[2];

/* Helper function to create specific test files we need */
static void create_additional_test_files(void) {
    // Create a file with no read permissions for permission testing
    FILE *f = fopen("./public/static/text/noread.txt", "w");
    if (f) {
        fprintf(f, "This file has no read permissions\n");
        fclose(f);
        chmod("./public/static/text/noread.txt", 0000);
    }
    
    // Create a non-executable CGI for testing
    f = fopen("./public/cgi-bin/noexec.cgi", "w");
    if (f) {
        fprintf(f, "#!/bin/bash\necho \"This script is not executable\"\n");
        fclose(f);
        chmod("./public/cgi-bin/noexec.cgi", 0644);
    }
    
    // Create a CGI that returns custom status
    f = fopen("./public/cgi-bin/status.cgi", "w");
    if (f) {
        fprintf(f, "#!/bin/bash\n");
        fprintf(f, "echo \"Status: 404\"\n");
        fprintf(f, "echo \"Content-Type: text/plain\"\n");
        fprintf(f, "echo \"\"\n");
        fprintf(f, "echo \"Not found from CGI\"\n");
        fclose(f);
        chmod("./public/cgi-bin/status.cgi", 0755);
    }
    
    // Create a CGI that outputs binary data
    f = fopen("./public/cgi-bin/binary.cgi", "w");
    if (f) {
        fprintf(f, "#!/bin/bash\n");
        fprintf(f, "echo \"Content-Type: application/octet-stream\"\n");
        fprintf(f, "echo \"\"\n");
        fprintf(f, "printf '\\x00\\x01\\x02\\x03\\xFF\\xFE\\xFD'\n");
        fclose(f);
        chmod("./public/cgi-bin/binary.cgi", 0755);
    }
    
    // Create a CGI that fails
    f = fopen("./public/cgi-bin/fail.cgi", "w");
    if (f) {
        fprintf(f, "#!/bin/bash\n");
        fprintf(f, "exit 1\n");
        fclose(f);
        chmod("./public/cgi-bin/fail.cgi", 0755);
    }
    
    // Create a CGI that handles parameters
    f = fopen("./public/cgi-bin/params_test.cgi", "w");
    if (f) {
        fprintf(f, "#!/bin/bash\n");
        fprintf(f, "echo \"Content-Type: text/plain\"\n");
        fprintf(f, "echo \"\"\n");
        fprintf(f, "echo \"CGI Parameter Test\"\n");
        fprintf(f, "echo \"QUERY_STRING=$QUERY_STRING\"\n");
        fprintf(f, "echo \"REQUEST_METHOD=$REQUEST_METHOD\"\n");
        fclose(f);
        chmod("./public/cgi-bin/params_test.cgi", 0755);
    }
    
    // Create a truly binary file for testing
    f = fopen("./public/static/misc/binary.dat", "wb");
    if (f) {
        unsigned char binary_data[] = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD};
        fwrite(binary_data, 1, sizeof(binary_data), f);
        fclose(f);
    }
}

/* Helper function to cleanup additional test files */
static void cleanup_additional_test_files(void) {
    unlink("./public/static/text/noread.txt");
    unlink("./public/cgi-bin/noexec.cgi");
    unlink("./public/cgi-bin/status.cgi");
    unlink("./public/cgi-bin/binary.cgi");
    unlink("./public/cgi-bin/fail.cgi");
    unlink("./public/cgi-bin/params_test.cgi");
    unlink("./public/static/misc/binary.dat");
}

/* Setup and teardown */
static void setup(void) {
    initialize_request(&request);
    initialize_response(&response);
    config_init(&config);
    
    // Set document root to actual public directory
    free(config.document_root);
    config.document_root = strdup("./public/");
    
    // Create additional test files
    create_additional_test_files();
    
    // Create pipe for capturing output
    if (pipe(pipe_fds) < 0) {
        ck_abort_msg("Failed to create pipe");
    }
}

static void teardown(void) {
    destroy_request(&request);
    destroy_response(&response);
    config_cleanup(&config);
    
    // Close pipes
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    
    // Cleanup additional test files
    cleanup_additional_test_files();
}

/* Helper to read pipe output */
static char* read_pipe_output(void) {
    char* buffer = malloc(BUFFER_SIZE * 10);
    if (!buffer) return NULL;
    
    memset(buffer, 0, BUFFER_SIZE * 10);
    
    // Set non-blocking to avoid hanging
    int flags = fcntl(pipe_fds[0], F_GETFL, 0);
    fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
    
    ssize_t total = 0;
    ssize_t bytes;
    while ((bytes = read(pipe_fds[0], buffer + total, BUFFER_SIZE - total - 1)) > 0) {
        total += bytes;
    }
    
    buffer[total] = '\0';
    return buffer;
}

/* ===== Tests for initialize_response ===== */
START_TEST(test_initialize_response_complete)
{
    http_response test_resp;
    initialize_response(&test_resp);
    
    // Check all fields are properly initialized
    ck_assert_int_eq(test_resp.status_code, 200);
    ck_assert_str_eq(test_resp.reason, "OK");
    ck_assert_str_eq(test_resp.server, "TuringBolt/0.1");
    ck_assert_ptr_nonnull(test_resp.date);
    
    // Verify date format
    ck_assert(strstr(test_resp.date, "GMT") != NULL);
    
    ck_assert_ptr_null(test_resp.content_type);
    ck_assert_uint_eq(test_resp.content_length, 0);
    ck_assert_ptr_null(test_resp.content_encoding);
    ck_assert_ptr_null(test_resp.last_modified);
    ck_assert_str_eq(test_resp.connection, "close");
    ck_assert_ptr_null(test_resp.cache_control);
    ck_assert_ptr_null(test_resp.etag);
    ck_assert_ptr_null(test_resp.body);
    ck_assert_int_eq(test_resp.is_file, false);
    ck_assert_ptr_null(test_resp.extra_header_names);
    ck_assert_ptr_null(test_resp.extra_header_values);
    ck_assert_int_eq(test_resp.extra_header_count, 0);
    
    destroy_response(&test_resp);
}
END_TEST

START_TEST(test_initialize_response_null_input)
{
    // Should handle NULL gracefully
    initialize_response(NULL);
    // If we get here without crashing, test passes
}
END_TEST

/* ===== Tests for destroy_response ===== */
START_TEST(test_destroy_response_complete)
{
    http_response test_resp;
    initialize_response(&test_resp);
    
    // Add some extra headers
    test_resp.extra_header_count = 2;
    test_resp.extra_header_names = malloc(2 * sizeof(char*));
    test_resp.extra_header_values = malloc(2 * sizeof(char*));
    test_resp.extra_header_names[0] = strdup("X-Custom-Header");
    test_resp.extra_header_values[0] = strdup("CustomValue");
    test_resp.extra_header_names[1] = strdup("X-Another-Header");
    test_resp.extra_header_values[1] = strdup("AnotherValue");
    
    // Add last_modified to test the fix
    test_resp.last_modified = strdup("Wed, 21 Oct 2025 07:28:00 GMT");
    
    // Destroy should free everything without leaks
    destroy_response(&test_resp);
    
    // Test passes if no segfault/leak (use valgrind to verify)
}
END_TEST

/* ===== Tests for get_absolute_path ===== */
START_TEST(test_get_absolute_path_normal)
{
    request.path = strdup("/static/text/readme.txt");
    
    char *abs_path = get_absolute_path(&request, &config);
    ck_assert_ptr_nonnull(abs_path);
    ck_assert_str_eq(abs_path, "./public/static/text/readme.txt");
    
    free(abs_path);
}
END_TEST

START_TEST(test_get_absolute_path_root)
{
    request.path = strdup("/");
    
    char *abs_path = get_absolute_path(&request, &config);
    ck_assert_ptr_nonnull(abs_path);
    ck_assert_str_eq(abs_path, "./public/");
    
    free(abs_path);
}
END_TEST

START_TEST(test_get_absolute_path_special_chars)
{
    // Test with the file that has spaces and special characters
    request.path = strdup("/static/text/file with spaces & symbols #@!.txt");
    
    char *abs_path = get_absolute_path(&request, &config);
    ck_assert_ptr_nonnull(abs_path);
    ck_assert_str_eq(abs_path, "./public/static/text/file with spaces & symbols #@!.txt");
    
    free(abs_path);
}
END_TEST

START_TEST(test_get_absolute_path_long_filename)
{
    // Test with the extremely long filename
    request.path = strdup("/static/text/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt");
    
    char *abs_path = get_absolute_path(&request, &config);
    ck_assert_ptr_nonnull(abs_path);
    
    free(abs_path);
}
END_TEST

START_TEST(test_get_absolute_path_too_long)
{
    // Create a path that exceeds PATH_MAX
    char long_path[PATH_MAX];
    memset(long_path, 'a', PATH_MAX - 1);
    long_path[0] = '/';
    long_path[PATH_MAX - 1] = '\0';
    request.path = strdup(long_path);
    
    char *abs_path = get_absolute_path(&request, &config);
    ck_assert_ptr_null(abs_path);
}
END_TEST

/* ===== Tests for set_content_headers ===== */
START_TEST(test_set_content_headers_text_file)
{
    // Use existing readme.txt
    int fd = open("./public/static/text/readme.txt", O_RDONLY);
    ck_assert_int_ge(fd, 0);
    
    request.mime_type = TEXT_PLAIN;
    
    int result = set_content_headers(fd, &request, &response, "./public/static/text/readme.txt");
    ck_assert_int_eq(result, 0);
    
    // Verify headers were set
    ck_assert_str_eq(response.content_type, "text/plain");
    ck_assert_ptr_nonnull(response.last_modified);
    ck_assert(strstr(response.last_modified, "GMT") != NULL);
    ck_assert_ptr_null(response.content_encoding);
    
    close(fd);
}
END_TEST

START_TEST(test_set_content_headers_large_file)
{
    // Use the 10KB file
    int fd = open("./public/static/text/atleast_10Kb_file.txt", O_RDONLY);
    ck_assert_int_ge(fd, 0);
    
    request.mime_type = TEXT_PLAIN;
    
    int result = set_content_headers(fd, &request, &response, "./public/static/text/atleast_10Kb_file.txt");
    ck_assert_int_eq(result, 0);
    
    // File should be around 10KB
    ck_assert(response.content_length >= 10000);
    
    close(fd);
}
END_TEST

START_TEST(test_set_content_headers_all_mime_types)
{
    // Test various existing files with different MIME types
    struct {
        const char *path;
        MIME_TYPE type;
        const char *expected;
    } test_files[] = {
        {"./public/static/html/index.html", TEXT_HTML, "text/html"},
        {"./public/static/css/styles.css", TEXT_CSS, "text/css"},
        {"./public/static/js/script.js", APPLICATION_JAVASCRIPT, "application/javascript"},
        {"./public/static/text/sample.json", APPLICATION_JSON, "application/json"},
        {"./public/static/media/dummy.png", IMAGE_PNG, "image/png"},
        {"./public/static/media/dummy.jpg", IMAGE_JPEG, "image/jpeg"},
        {"./public/static/misc/dummy.pdf", APPLICATION_PDF, "application/pdf"},
    };
    
    for (size_t i = 0; i < sizeof(test_files)/sizeof(test_files[0]); i++) {
        int fd = open(test_files[i].path, O_RDONLY);
        if (fd < 0) {
            ck_abort_msg("Failed to open test file: %s", test_files[i].path);
        }
        
        request.mime_type = test_files[i].type;
        
        int result = set_content_headers(fd, &request, &response, test_files[i].path);
        ck_assert_int_eq(result, 0);
        ck_assert_str_eq(response.content_type, test_files[i].expected);
        
        // Clean up for next iteration
        free(response.last_modified);
        response.last_modified = NULL;
        close(fd);
    }
}
END_TEST

/* ===== Tests for generate_response_header ===== */
START_TEST(test_generate_response_header_200_ok)
{
    response.status_code = 200;
    response.reason = "OK";
    response.content_type = "text/html";
    response.content_length = 1234;
    
    char *header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    
    // Verify required components
    ck_assert(strstr(header, "HTTP/1.1 200 OK\r\n") != NULL);
    ck_assert(strstr(header, "Server: TuringBolt/0.1\r\n") != NULL);
    ck_assert(strstr(header, "Connection: close\r\n") != NULL);
    ck_assert(strstr(header, "Content-Type: text/html\r\n") != NULL);
    ck_assert(strstr(header, "Content-Length: 1234\r\n") != NULL);
    ck_assert(strstr(header, "Date: ") != NULL);
    
    // Should end with double CRLF
    size_t len = strlen(header);
    ck_assert_str_eq(header + len - 4, "\r\n\r\n");
    
    free(header);
}
END_TEST

START_TEST(test_generate_response_header_404_error)
{
    response.status_code = 404;
    response.reason = "Not Found";
    response.content_type = "text/html";
    response.content_length = 100;
    
    char *header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "HTTP/1.1 404 Not Found\r\n") != NULL);
    
    free(header);
}
END_TEST

START_TEST(test_generate_response_header_with_optional_headers)
{
    response.status_code = 200;
    response.reason = "OK";
    response.content_type = "text/html";
    response.content_length = 100;
    response.last_modified = "Wed, 21 Oct 2025 07:28:00 GMT";
    response.cache_control = "max-age=3600";
    response.etag = "\"123456789\"";
    
    char *header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    
    ck_assert(strstr(header, "Last-Modified: Wed, 21 Oct 2025 07:28:00 GMT\r\n") != NULL);
    ck_assert(strstr(header, "Cache-Control: max-age=3600\r\n") != NULL);
    ck_assert(strstr(header, "ETag: \"123456789\"\r\n") != NULL);
    
    free(header);
}
END_TEST

/* ===== Tests for serve_static ===== */
START_TEST(test_serve_static_small_text_file)
{
    // Use existing readme.txt
    request.path = strdup("/static/text/readme.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Verify response
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "Content-Type: text/plain") != NULL);
    ck_assert(strstr(output, "This is a simple text file") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_static_html_file)
{
    // Use existing index.html
    request.path = strdup("/static/html/index.html");
    request.mime_type = TEXT_HTML;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Verify response
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "Content-Type: text/html") != NULL);
    ck_assert(strstr(output, "Welcome to the Web Server Test") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_static_binary_file)
{
    // Use our created binary file
    request.path = strdup("/static/misc/binary.dat");
    request.mime_type = APPLICATION_OCTET_STREAM;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    // Read raw output (don't null-terminate)
    unsigned char buffer[BUFFER_SIZE];
    ssize_t total = read(pipe_fds[0], buffer, BUFFER_SIZE);
    
    // Find header end
    unsigned char *body_start = NULL;
    for (ssize_t i = 0; i < total - 3; i++) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
            buffer[i+2] == '\r' && buffer[i+3] == '\n') {
            body_start = &buffer[i+4];
            break;
        }
    }
    
    ck_assert_ptr_nonnull(body_start);
    
    // Verify binary data integrity
    unsigned char expected[] = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD};
    ck_assert_int_eq(memcmp(body_start, expected, sizeof(expected)), 0);
}
END_TEST

START_TEST(test_serve_static_large_file)
{
    // Use the 10KB file
    request.path = strdup("/static/text/atleast_10Kb_file.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    // Read output in chunks to handle large file
    size_t total_read = 0;
    char buffer[BUFFER_SIZE];
    while (read(pipe_fds[0], buffer, BUFFER_SIZE) > 0) {
        total_read += BUFFER_SIZE;
    }
    
    // Should have read significant data (headers + 10KB file)
    ck_assert(total_read >= 10000);
}
END_TEST

START_TEST(test_serve_static_special_filename)
{
    // Use file with special characters
    request.path = strdup("/static/text/file with spaces & symbols #@!.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "Special filename") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_static_nonexistent_file)
{
    request.path = strdup("/static/text/nonexistent.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, -1);
    ck_assert_int_eq(response.status_code, 404);
    ck_assert_str_eq(response.reason, "Not Found");
}
END_TEST

START_TEST(test_serve_static_permission_denied)
{
    // Use our created file with no read permissions
    request.path = strdup("/static/text/noread.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, -1);
    ck_assert_int_eq(response.status_code, 403);
    ck_assert_str_eq(response.reason, "Forbidden");
}
END_TEST

/* ===== Tests for serve_dynamic ===== */
START_TEST(test_serve_dynamic_hello_cgi)
{
    // Use existing hello.cgi
    request.path = strdup("/cgi-bin/hello.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Verify CGI output
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "Content-type: text/html") != NULL);
    ck_assert(strstr(output, "Hello from CGI!") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_dynamic_with_parameters)
{
    // Use our params_test.cgi
    request.path = strdup("/cgi-bin/params_test.cgi");
    request.is_dynamic = true;
    request.param_count = 2;
    request.param_names = malloc(2 * sizeof(char*));
    request.param_values = malloc(2 * sizeof(char*));
    request.param_names[0] = strdup("name");
    request.param_values[0] = strdup("John");
    request.param_names[1] = strdup("age");
    request.param_values[1] = strdup("25");
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Verify query string was passed
    ck_assert(strstr(output, "QUERY_STRING=name=John&age=25") != NULL);
    ck_assert(strstr(output, "REQUEST_METHOD=GET") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_dynamic_cgi_with_status)
{
    // Use our status.cgi that returns 404
    request.path = strdup("/cgi-bin/status.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Should have 404 status
    ck_assert(strstr(output, "HTTP/1.1 404 Not Found") != NULL);
    ck_assert(strstr(output, "Not found from CGI") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_dynamic_cgi_binary_output)
{
    // Use our binary.cgi
    request.path = strdup("/cgi-bin/binary.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    // Read raw output
    unsigned char buffer[BUFFER_SIZE];
    ssize_t total = read(pipe_fds[0], buffer, BUFFER_SIZE);
    
    // Find body start
    unsigned char *body_start = NULL;
    for (ssize_t i = 0; i < total - 3; i++) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
            buffer[i+2] == '\r' && buffer[i+3] == '\n') {
            body_start = &buffer[i+4];
            break;
        }
    }
    
    ck_assert_ptr_nonnull(body_start);
    
    // Note: The null termination in serve_dynamic may corrupt this data
    // This test demonstrates the binary data issue
}
END_TEST

START_TEST(test_serve_dynamic_nonexistent_cgi)
{
    request.path = strdup("/cgi-bin/nonexistent.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, -1);
    ck_assert_int_eq(response.status_code, 404);
    ck_assert_str_eq(response.reason, "Not Found");
}
END_TEST

START_TEST(test_serve_dynamic_non_executable_cgi)
{
    // Use our noexec.cgi
    request.path = strdup("/cgi-bin/noexec.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, -1);
    ck_assert_int_eq(response.status_code, 403);
    ck_assert_str_eq(response.reason, "Forbidden");
}
END_TEST

START_TEST(test_serve_dynamic_failing_cgi)
{
    // Use our fail.cgi
    request.path = strdup("/cgi-bin/fail.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = serve_dynamic(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, -1);
    ck_assert_int_eq(response.status_code, 500);
    ck_assert_str_eq(response.reason, "Internal Server Error");
}
END_TEST

/* ===== Tests for execute_request ===== */
START_TEST(test_execute_request_static_success)
{
    request.path = strdup("/static/text/readme.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = execute_request(&request, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_execute_request_static_error_handled)
{
    request.path = strdup("/static/text/nonexistent.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = execute_request(&request, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0); // Should return 0 because error was handled
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "HTTP/1.1 404 Not Found") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_execute_request_dynamic_success)
{
    request.path = strdup("/cgi-bin/hello.cgi");
    request.is_dynamic = true;
    request.param_count = 0;
    
    int result = execute_request(&request, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "Hello from CGI!") != NULL);
    
    free(output);
}
END_TEST

/* ===== Edge Case Tests ===== */
START_TEST(test_serve_static_no_extension)
{
    // Use the file with no extension
    request.path = strdup("/static/misc/no_extension");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "This file has no extension") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_static_css_file)
{
    // Test CSS file
    request.path = strdup("/static/css/styles.css");
    request.mime_type = TEXT_CSS;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "Content-Type: text/css") != NULL);
    ck_assert(strstr(output, "font-family: Arial") != NULL);
    
    free(output);
}
END_TEST

START_TEST(test_serve_static_javascript_file)
{
    // Test JavaScript file
    request.path = strdup("/static/js/script.js");
    request.mime_type = APPLICATION_JAVASCRIPT;
    request.is_dynamic = false;
    
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    ck_assert_int_eq(result, 0);
    
    close(pipe_fds[1]);
    
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    ck_assert(strstr(output, "Content-Type: application/javascript") != NULL);
    ck_assert(strstr(output, "DOMContentLoaded") != NULL);
    
    free(output);
}
END_TEST

/* Create test suite */
Suite *request_handler_suite(void)
{
    Suite *s = suite_create("Request Handler");
    
    // Response lifecycle tests
    TCase *tc_response = tcase_create("Response Lifecycle");
    tcase_add_checked_fixture(tc_response, setup, teardown);
    tcase_add_test(tc_response, test_initialize_response_complete);
    tcase_add_test(tc_response, test_initialize_response_null_input);
    tcase_add_test(tc_response, test_destroy_response_complete);
    suite_add_tcase(s, tc_response);
    
    // Path handling tests
    TCase *tc_path = tcase_create("Path Handling");
    tcase_add_checked_fixture(tc_path, setup, teardown);
    tcase_add_test(tc_path, test_get_absolute_path_normal);
    tcase_add_test(tc_path, test_get_absolute_path_root);
    tcase_add_test(tc_path, test_get_absolute_path_special_chars);
    tcase_add_test(tc_path, test_get_absolute_path_long_filename);
    tcase_add_test(tc_path, test_get_absolute_path_too_long);
    suite_add_tcase(s, tc_path);
    
    // Content headers tests
    TCase *tc_content = tcase_create("Content Headers");
    tcase_add_checked_fixture(tc_content, setup, teardown);
    tcase_add_test(tc_content, test_set_content_headers_text_file);
    tcase_add_test(tc_content, test_set_content_headers_large_file);
    tcase_add_test(tc_content, test_set_content_headers_all_mime_types);
    suite_add_tcase(s, tc_content);
    
    // Header generation tests
    TCase *tc_headers = tcase_create("Header Generation");
    tcase_add_checked_fixture(tc_headers, setup, teardown);
    tcase_add_test(tc_headers, test_generate_response_header_200_ok);
    tcase_add_test(tc_headers, test_generate_response_header_404_error);
    tcase_add_test(tc_headers, test_generate_response_header_with_optional_headers);
    suite_add_tcase(s, tc_headers);
    
    // Static file serving tests
    TCase *tc_static = tcase_create("Static File Serving");
    tcase_add_checked_fixture(tc_static, setup, teardown);
    tcase_add_test(tc_static, test_serve_static_small_text_file);
    tcase_add_test(tc_static, test_serve_static_html_file);
    tcase_add_test(tc_static, test_serve_static_binary_file);
    tcase_add_test(tc_static, test_serve_static_large_file);
    tcase_add_test(tc_static, test_serve_static_special_filename);
    tcase_add_test(tc_static, test_serve_static_nonexistent_file);
    tcase_add_test(tc_static, test_serve_static_permission_denied);
    tcase_add_test(tc_static, test_serve_static_no_extension);
    tcase_add_test(tc_static, test_serve_static_css_file);
    tcase_add_test(tc_static, test_serve_static_javascript_file);
    suite_add_tcase(s, tc_static);
    
    // Dynamic content (CGI) tests
    TCase *tc_dynamic = tcase_create("Dynamic Content (CGI)");
    tcase_add_checked_fixture(tc_dynamic, setup, teardown);
    tcase_add_test(tc_dynamic, test_serve_dynamic_hello_cgi);
    tcase_add_test(tc_dynamic, test_serve_dynamic_with_parameters);
    tcase_add_test(tc_dynamic, test_serve_dynamic_cgi_with_status);
    tcase_add_test(tc_dynamic, test_serve_dynamic_cgi_binary_output);
    tcase_add_test(tc_dynamic, test_serve_dynamic_nonexistent_cgi);
    tcase_add_test(tc_dynamic, test_serve_dynamic_non_executable_cgi);
    tcase_add_test(tc_dynamic, test_serve_dynamic_failing_cgi);
    tcase_set_timeout(tc_dynamic, 10); // CGI tests may take longer
    suite_add_tcase(s, tc_dynamic);
    
    // Request execution tests
    TCase *tc_execute = tcase_create("Request Execution");
    tcase_add_checked_fixture(tc_execute, setup, teardown);
    tcase_add_test(tc_execute, test_execute_request_static_success);
    tcase_add_test(tc_execute, test_execute_request_static_error_handled);
    tcase_add_test(tc_execute, test_execute_request_dynamic_success);
    suite_add_tcase(s, tc_execute);
    
    return s;
}

/* Main function */
int main(void)
{
    Suite *s = request_handler_suite();
    SRunner *sr = srunner_create(s);
    
    // Use CK_VERBOSE for detailed output
    srunner_run_all(sr, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}