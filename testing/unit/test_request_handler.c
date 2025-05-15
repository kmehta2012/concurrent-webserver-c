#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "request_handler.h"
#include "http_parser.h"
#include "config.h"
#include "logger.h"

/* Test fixtures */
static http_request request;
static http_response response;
static server_config config;
static int pipe_fds[2];  // For capturing socket output

/* Setup and teardown */
static void setup(void) {
    initialize_request(&request);
    initialize_response(&response);
    config_init(&config);
    
    // Create a pipe to capture socket output
    if (pipe(pipe_fds) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
}

static void teardown(void) {
    destroy_request(&request);
    destroy_response(&response);
    config_cleanup(&config);
    
    // Close pipe
    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

/* Helper functions */
static char* read_pipe_output(void) {
    char* buffer = malloc(8192);
    if (!buffer) return NULL;
    
    memset(buffer, 0, 8192);
    
    // Read from pipe read end
    ssize_t bytes_read = read(pipe_fds[0], buffer, 8191);
    if (bytes_read < 0) {
        free(buffer);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    return buffer;
}

/* Test cases for initialize_response */
START_TEST(test_initialize_response_sets_defaults)
{
    http_response test_response;
    initialize_response(&test_response);
    
    ck_assert_int_eq(test_response.status_code, 200);
    ck_assert_str_eq(test_response.reason, "OK");
    ck_assert_str_eq(test_response.server, "TuringBolt/0.1");
    ck_assert_ptr_nonnull(test_response.date);
    ck_assert_ptr_null(test_response.content_type);
    ck_assert_int_eq(test_response.content_length, 0);
    ck_assert_ptr_null(test_response.content_encoding);
    ck_assert_ptr_null(test_response.last_modified);
    ck_assert_str_eq(test_response.connection, "close");
    ck_assert_ptr_null(test_response.cache_control);
    ck_assert_ptr_null(test_response.etag);
    ck_assert_ptr_null(test_response.body);
    ck_assert_int_eq(test_response.is_file, false);
    ck_assert_ptr_null(test_response.extra_header_names);
    ck_assert_ptr_null(test_response.extra_header_values);
    ck_assert_int_eq(test_response.extra_header_count, 0);
    
    destroy_response(&test_response);
}
END_TEST

/* Test cases for get_absolute_path */
START_TEST(test_get_absolute_path_normal_path)
{
    // Setup
    config.document_root = strdup("./public/");
    request.path = strdup("/static/test.html");
    
    // Test
    char *result = get_absolute_path(&request, &config);
    
    // Verify
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "./public/static/test.html");
    
    // Cleanup
    free(result);
}
END_TEST

START_TEST(test_get_absolute_path_empty_path)
{
    // Setup
    config.document_root = strdup("./public/");
    request.path = strdup("");
    
    // Test
    char *result = get_absolute_path(&request, &config);
    
    // Verify
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "./public/");
    
    // Cleanup
    free(result);
}
END_TEST

/* Test cases for mime_type_to_string */
START_TEST(test_mime_type_to_string)
{
    // This function is static, so we need to test it indirectly through other functions
    // We'll use generate_response_header to test it
    
    response.status_code = 200;
    response.reason = "OK";
    
    // Test HTML
    response.content_type = "text/html";
    char *header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "Content-Type: text/html") != NULL);
    free(header);
    
    // Test Plain Text
    response.content_type = "text/plain";
    header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "Content-Type: text/plain") != NULL);
    free(header);
    
    // Test JPEG
    response.content_type = "image/jpeg";
    header = generate_response_header(&response);
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "Content-Type: image/jpeg") != NULL);
    free(header);
}
END_TEST

/* Test cases for generate_response_header */
START_TEST(test_generate_response_header_success)
{
    // Setup
    response.status_code = 200;
    response.reason = "OK";
    response.content_type = "text/html";
    response.content_length = 100;
    
    // Test
    char *header = generate_response_header(&response);
    
    // Verify
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(header, "Content-Type: text/html") != NULL);
    ck_assert(strstr(header, "Content-Length: 100") != NULL);
    ck_assert(strstr(header, "Server: TuringBolt/0.1") != NULL);
    
    // Cleanup
    free(header);
}
END_TEST

START_TEST(test_generate_response_header_error)
{
    // Setup
    response.status_code = 404;
    response.reason = "Not Found";
    response.content_type = "text/html";
    response.content_length = 100;
    
    // Test
    char *header = generate_response_header(&response);
    
    // Verify
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "HTTP/1.1 404 Not Found") != NULL);
    
    // Cleanup
    free(header);
}
END_TEST

START_TEST(test_generate_response_header_with_headers)
{
    // Setup
    response.status_code = 200;
    response.reason = "OK";
    response.content_type = "text/html";
    response.content_length = 100;
    response.last_modified = "Wed, 14 May 2025 12:00:00 GMT";
    response.cache_control = "max-age=3600";
    
    // Test
    char *header = generate_response_header(&response);
    
    // Verify
    ck_assert_ptr_nonnull(header);
    ck_assert(strstr(header, "Last-Modified: Wed, 14 May 2025 12:00:00 GMT") != NULL);
    ck_assert(strstr(header, "Cache-Control: max-age=3600") != NULL);
    
    // Cleanup
    free(header);
}
END_TEST

/* Test cases for set_content_headers */
START_TEST(test_set_content_headers)
{
    // This test requires a real file since it calls fstat
    
    // Setup
    char file_path[] = "./public/static/text/readme.txt";
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        ck_abort_msg("Test file not found: %s", file_path);
    }
    
    request.mime_type = TEXT_PLAIN;
    
    // Test
    int result = set_content_headers(fd, &request, &response, file_path);
    
    // Verify
    ck_assert_int_eq(result, 0);
    ck_assert_str_eq(response.content_type, "text/plain");
    ck_assert_ptr_nonnull(response.last_modified);
    ck_assert(response.content_length > 0);
    
    // Cleanup
    close(fd);
}
END_TEST

START_TEST(test_set_content_headers_invalid_fd)
{
    // Setup - use an invalid file descriptor
    int invalid_fd = -1;
    request.mime_type = TEXT_PLAIN;
    
    // Test
    int result = set_content_headers(invalid_fd, &request, &response, "invalid.txt");
    
    // Verify
    ck_assert_int_eq(result, -1);
    
    // No cleanup needed
}
END_TEST

/* Test cases for serve_static */
START_TEST(test_serve_static_existing_file)
{
    // Setup
    config.document_root = strdup("./public/");
    request.path = strdup("/static/text/readme.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    // Test - use the pipe write end as the client socket
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    
    // Verify
    ck_assert_int_eq(result, 0);
    
    // Read what was sent to the "client"
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Should contain HTTP response line and headers
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    ck_assert(strstr(output, "Content-Type: text/plain") != NULL);
    
    // Should contain file content
    ck_assert(strstr(output, "This is a simple text file") != NULL);
    
    // Cleanup
    free(output);
}
END_TEST

START_TEST(test_serve_static_missing_file)
{
    // Setup
    config.document_root = strdup("./public/");
    request.path = strdup("/static/nonexistent.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    // Test - use the pipe write end as the client socket
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    
    // Verify - should return -1 for error
    ck_assert_int_eq(result, -1);
    
    // Response should have 404 status
    ck_assert_int_eq(response.status_code, 404);
    ck_assert_str_eq(response.reason, "Not Found");
    
    // Read what was sent to the "client"
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Should have sent an error response
    ck_assert(strstr(output, "HTTP/1.1 404 Not Found") != NULL);
    
    // Cleanup
    free(output);
}
END_TEST

START_TEST(test_serve_static_special_chars_in_filename)
{
    // Setup
    config.document_root = strdup("./public/");
    // Use a file with spaces and special characters
    request.path = strdup("/static/text/file with spaces & symbols #@!.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    // Test
    int result = serve_static(&request, &response, pipe_fds[1], &config);
    
    // Verify
    ck_assert_int_eq(result, 0);
    
    // Read what was sent to the "client"
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Should contain HTTP 200 response
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    
    // Should contain file content
    ck_assert(strstr(output, "Special filename") != NULL);
    
    // Cleanup
    free(output);
}
END_TEST

START_TEST(test_serve_static_different_mime_types)
{
    // Array of paths and expected MIME types
    const char *test_files[][2] = {
        {"/static/html/index.html", "text/html"},
        {"/static/css/styles.css", "text/css"},
        {"/static/text/sample.json", "application/json"},
        {"/static/media/dummy.png", "image/png"},
        {"/static/misc/dummy.pdf", "application/pdf"}
    };
    
    // Setup common config
    config.document_root = strdup("./public/");
    
    for (size_t i = 0; i < sizeof(test_files) / sizeof(test_files[0]); i++) {
        // Setup for this iteration
        request.path = strdup(test_files[i][0]);
        request.mime_type = get_mime_type(request.path);
        request.is_dynamic = false;
        
        // Reset pipe for clean output
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        pipe(pipe_fds);
        
        // Test
        int result = serve_static(&request, &response, pipe_fds[1], &config);
        
        // Verify
        ck_assert_int_eq(result, 0);
        
        // Read what was sent to the "client"
        char *output = read_pipe_output();
        ck_assert_ptr_nonnull(output);
        
        // Construct expected Content-Type header
        char expected_header[50];
        sprintf(expected_header, "Content-Type: %s", test_files[i][1]);
        
        // Verify correct MIME type in header
        ck_assert_msg(strstr(output, expected_header) != NULL, 
                      "MIME type mismatch for %s, expected: %s", 
                      test_files[i][0], expected_header);
        
        // Cleanup this iteration
        free(output);
        free(request.path);
    }
}
END_TEST

/* Test cases for execute_request */
START_TEST(test_execute_request_static)
{
    // Setup
    config.document_root = strdup("./public/");
    request.path = strdup("/static/text/readme.txt");
    request.mime_type = TEXT_PLAIN;
    request.is_dynamic = false;
    
    // Test
    int result = execute_request(&request, pipe_fds[1], &config);
    
    // Verify
    ck_assert_int_eq(result, 0);
    
    // Read what was sent to the "client"
    char *output = read_pipe_output();
    ck_assert_ptr_nonnull(output);
    
    // Should contain HTTP 200 response
    ck_assert(strstr(output, "HTTP/1.1 200 OK") != NULL);
    
    // Cleanup
    free(output);
}
END_TEST

/* Create test suite */
Suite *request_handler_suite(void)
{
    Suite *s = suite_create("Request Handler");
    
    // Test case for response initialization and cleanup
    TCase *tc_response = tcase_create("Response Management");
    tcase_add_checked_fixture(tc_response, setup, teardown);
    tcase_add_test(tc_response, test_initialize_response_sets_defaults);
    suite_add_tcase(s, tc_response);
    
    // Test case for path handling
    TCase *tc_path = tcase_create("Path Handling");
    tcase_add_checked_fixture(tc_path, setup, teardown);
    tcase_add_test(tc_path, test_get_absolute_path_normal_path);
    tcase_add_test(tc_path, test_get_absolute_path_empty_path);
    suite_add_tcase(s, tc_path);
    
    // Test case for MIME type handling
    TCase *tc_mime = tcase_create("MIME Types");
    tcase_add_checked_fixture(tc_mime, setup, teardown);
    tcase_add_test(tc_mime, test_mime_type_to_string);
    suite_add_tcase(s, tc_mime);
    
    // Test case for header generation
    TCase *tc_headers = tcase_create("Header Generation");
    tcase_add_checked_fixture(tc_headers, setup, teardown);
    tcase_add_test(tc_headers, test_generate_response_header_success);
    tcase_add_test(tc_headers, test_generate_response_header_error);
    tcase_add_test(tc_headers, test_generate_response_header_with_headers);
    tcase_add_test(tc_headers, test_set_content_headers);
    tcase_add_test(tc_headers, test_set_content_headers_invalid_fd);
    suite_add_tcase(s, tc_headers);
    
    // Test case for file serving
    TCase *tc_serve = tcase_create("Static File Serving");
    tcase_add_checked_fixture(tc_serve, setup, teardown);
    tcase_add_test(tc_serve, test_serve_static_existing_file);
    tcase_add_test(tc_serve, test_serve_static_missing_file);
    tcase_add_test(tc_serve, test_serve_static_special_chars_in_filename);
    tcase_add_test(tc_serve, test_serve_static_different_mime_types);
    suite_add_tcase(s, tc_serve);
    
    // Test case for request execution
    TCase *tc_execute = tcase_create("Request Execution");
    tcase_add_checked_fixture(tc_execute, setup, teardown);
    tcase_add_test(tc_execute, test_execute_request_static);
    suite_add_tcase(s, tc_execute);
    
    return s;
}

/* Main function */
int main(void)
{
    Suite *s = request_handler_suite();
    SRunner *sr = srunner_create(s);
    
    // Use CK_VERBOSE for detailed output, CK_NORMAL for normal output
    srunner_run_all(sr, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}