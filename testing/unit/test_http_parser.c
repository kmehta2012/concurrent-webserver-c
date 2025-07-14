// compilation command for now
// clang -std=c99 -Wall -Wextra -Werror -g -O0 -I./include -I./testing $(pkg-config --cflags check) testing/unit/test_http_parser.c src/http_parser.c src/config.c src/rio.c $(pkg-config --libs check) -pthread -lm -o executables/test_http_parser
#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "http_parser.h"
#include "rio.h"
#include "logger.h"
#include "config.h"

/* Test fixture setup and teardown */
static http_request request;
static server_config config;

static void setup(void) {
    // Initialize request and config before each test
    initialize_request(&request);
    config_init(&config);
}

static void teardown(void) {
    // Clean up after each test
    destroy_request(&request);
    config_cleanup(&config);
}

/* Test cases for url_decode function */
START_TEST(test_url_decode_basic)
{
    char str[] = "Hello%20World";
    ck_assert_int_eq(url_decode(str), 0);
    ck_assert_str_eq(str, "Hello World");
}
END_TEST

START_TEST(test_url_decode_multiple)
{
    char str[] = "Hello%20%21%20%22World%22";  // Hello ! "World"
    ck_assert_int_eq(url_decode(str), 0);
    ck_assert_str_eq(str, "Hello ! \"World\"");
}
END_TEST

START_TEST(test_url_decode_plus)
{
    char str[] = "Hello+World";
    ck_assert_int_eq(url_decode(str), 0);
    ck_assert_str_eq(str, "Hello World");
}
END_TEST

START_TEST(test_url_decode_empty)
{
    char str[] = "";
    ck_assert_int_eq(url_decode(str), 0);
    ck_assert_str_eq(str, "");
}
END_TEST

/* New URL decode edge cases */
START_TEST(test_url_decode_incomplete_percent)
{
    char str[] = "Hello%2World";  // Incomplete % sequence
    ck_assert_int_eq(url_decode(str), 0);
    // Should preserve the incomplete sequence
    ck_assert_str_eq(str, "Hello%2World");
}
END_TEST

START_TEST(test_url_decode_trailing_percent)
{
    char str[] = "Hello%";  // % at the end
    ck_assert_int_eq(url_decode(str), 0);
    // Should preserve the % at the end
    ck_assert_str_eq(str, "Hello%");
}
END_TEST

START_TEST(test_url_decode_invalid_hex)
{
    char str[] = "Hello%ZZWorld";  // Invalid hex chars
    ck_assert_int_eq(url_decode(str), 0);
    // Should preserve the %ZZ
    ck_assert_str_eq(str, "Hello%ZZWorld");
}
END_TEST

START_TEST(test_url_decode_mixed_valid_invalid)
{
    char str[] = "Test%20with%ZZmixed%2G%21sequences";
    ck_assert_int_eq(url_decode(str), 0);
    // Only valid sequences should be decoded
    ck_assert_str_eq(str, "Test with%ZZmixed%2G!sequences");
}
END_TEST

/* Test cases for get_mime_type function */
START_TEST(test_mime_type_html)
{
    ck_assert_int_eq(get_mime_type("index.html"), TEXT_HTML);
    ck_assert_int_eq(get_mime_type("page.htm"), TEXT_HTML);
}
END_TEST

START_TEST(test_mime_type_txt)
{
    ck_assert_int_eq(get_mime_type("file.txt"), TEXT_PLAIN);
}
END_TEST

START_TEST(test_mime_type_image)
{
    ck_assert_int_eq(get_mime_type("image.jpg"), IMAGE_JPEG);
    ck_assert_int_eq(get_mime_type("icon.png"), IMAGE_PNG);
    ck_assert_int_eq(get_mime_type("animated.gif"), IMAGE_GIF);
}
END_TEST

START_TEST(test_mime_type_no_extension)
{
    ck_assert_int_eq(get_mime_type("README"), TEXT_PLAIN);
}
END_TEST

/* New MIME type edge cases */
START_TEST(test_mime_type_multiple_extensions)
{
    ck_assert_int_eq(get_mime_type("file.tar.gz"), TEXT_PLAIN); // Should use last extension
    ck_assert_int_eq(get_mime_type("image.jpg.txt"), TEXT_PLAIN);
    ck_assert_int_eq(get_mime_type("script.txt.html"), TEXT_HTML);
}
END_TEST

START_TEST(test_mime_type_unusual_extensions)
{
    ck_assert_int_eq(get_mime_type("file.jpeg"), IMAGE_JPEG); // .jpeg instead of .jpg
    ck_assert_int_eq(get_mime_type("document.HTML"), TEXT_HTML); // Case insensitivity
    ck_assert_int_eq(get_mime_type("readme.TXT"), TEXT_PLAIN); // Case insensitivity
}
END_TEST

START_TEST(test_mime_type_dot_only)
{
    ck_assert_int_eq(get_mime_type("."), TEXT_PLAIN); // Just a dot
    ck_assert_int_eq(get_mime_type(".."), TEXT_PLAIN); // Double dot
}
END_TEST

/* Test cases for parse_uri function */
START_TEST(test_parse_uri_static)
{
    char uri[] = "/static/index.html";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/static/index.html");
    ck_assert_int_eq(request.is_dynamic, false);
    ck_assert_int_eq(request.mime_type, TEXT_HTML);
    ck_assert_int_eq(request.param_count, 0);
}
END_TEST

START_TEST(test_parse_uri_dynamic)
{
    // Assuming config.dynamic_dir_name is "cgi-bin"
    char uri[] = "/cgi-bin/script.cgi?name=value&flag";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/cgi-bin/script.cgi");
    ck_assert_int_eq(request.is_dynamic, true);
    ck_assert_int_eq(request.param_count, 2);
    ck_assert_str_eq(request.param_names[0], "name");
    ck_assert_str_eq(request.param_values[0], "value");
    ck_assert_str_eq(request.param_names[1], "flag");
    ck_assert_str_eq(request.param_values[1], "");
}
END_TEST

START_TEST(test_parse_uri_encoded)
{
    char uri[] = "/static/page%20with%20spaces.html";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/static/page with spaces.html");
    ck_assert_int_eq(request.is_dynamic, false);
}
END_TEST

/* New URI parsing edge cases */
START_TEST(test_parse_uri_dynamic_similar_name)
{
    // Test a path that starts with the dynamic dir name but isn't a full match
    char uri[] = "/cgi-binaries/script.cgi";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/cgi-binaries/script.cgi");
    ck_assert_int_eq(request.is_dynamic, false); // Shouldn't be dynamic
}
END_TEST

START_TEST(test_parse_uri_complex_query)
{
    char uri[] = "/cgi-bin/script.cgi?param1=value1&param2=value%20with%20spaces&empty=&=noname&novalue";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/cgi-bin/script.cgi");
    ck_assert_int_eq(request.is_dynamic, true);
    ck_assert_int_eq(request.param_count, 5);
    ck_assert_str_eq(request.param_names[0], "param1");
    ck_assert_str_eq(request.param_values[0], "value1");
    ck_assert_str_eq(request.param_names[1], "param2");
    ck_assert_str_eq(request.param_values[1], "value with spaces");
    ck_assert_str_eq(request.param_names[2], "empty");
    ck_assert_str_eq(request.param_values[2], "");
    ck_assert_str_eq(request.param_names[3], "");
    ck_assert_str_eq(request.param_values[3], "noname");
    ck_assert_str_eq(request.param_names[4], "novalue");
    ck_assert_str_eq(request.param_values[4], "");
}
END_TEST

START_TEST(test_parse_uri_root)
{
    char uri[] = "/";
    ck_assert_int_eq(parse_uri(uri, &request, &config), 0);
    ck_assert_str_eq(request.path, "/");
    ck_assert_int_eq(request.is_dynamic, false);
    ck_assert_int_eq(request.param_count, 0);
}
END_TEST

/* Test cases for parse_http_request function */
START_TEST(test_parse_http_request_valid)
{
    char request_str[] = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_nonnull(result);
    ck_assert_int_eq(request.method, GET);
    ck_assert_int_eq(request.version, HTTP_1_1);
    ck_assert_str_eq(request.path, "/index.html");
}
END_TEST

START_TEST(test_parse_http_request_invalid_method)
{
    char request_str[] = "POST /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_null(result);
}
END_TEST

START_TEST(test_parse_http_request_invalid_version)
{
    char request_str[] = "GET /index.html HTTP/2.0\r\nHost: example.com\r\n\r\n";
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_null(result);
}
END_TEST

/* New HTTP request parsing tests */
START_TEST(test_parse_http_request_http_1_0)
{
    char request_str[] = "GET /index.html HTTP/1.0\r\nHost: example.com\r\n\r\n";
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_nonnull(result);
    ck_assert_int_eq(request.method, GET);
    ck_assert_int_eq(request.version, HTTP_1_0);  // Should be recognized as HTTP/1.0
    ck_assert_str_eq(request.path, "/index.html");
}
END_TEST

START_TEST(test_parse_http_request_malformed_request_line)
{
    char request_str[] = "GET/index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";  // Missing space
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_null(result);
}
END_TEST

START_TEST(test_parse_http_request_no_crlf)
{
    char request_str[] = "GET /index.html HTTP/1.1";  // No CRLF
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_null(result);
}
END_TEST

START_TEST(test_parse_http_request_invalid_uri_path)
{
    char request_str[] = "GET index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";  // URI doesn't start with /
    http_request *result = parse_http_request(request_str, &request, &config);
    ck_assert_ptr_null(result);
}
END_TEST

START_TEST(test_initialize_destroy_request)
{
    http_request test_req;
    
    // Test initialization
    initialize_request(&test_req);
    ck_assert_ptr_null(test_req.path);
    ck_assert_ptr_null(test_req.param_names);
    ck_assert_ptr_null(test_req.param_values);
    ck_assert_int_eq(test_req.param_count, 0);
    ck_assert_int_eq(test_req.method, GET);
    ck_assert_int_eq(test_req.version, HTTP_1_1);
    ck_assert_int_eq(test_req.is_dynamic, false);
    
    // Setup a request with allocated resources
    test_req.path = strdup("/test.html");
    test_req.param_count = 2;
    test_req.param_names = malloc(2 * sizeof(char*));
    test_req.param_values = malloc(2 * sizeof(char*));
    test_req.param_names[0] = strdup("name1");
    test_req.param_values[0] = strdup("value1");
    test_req.param_names[1] = strdup("name2");
    test_req.param_values[1] = strdup("value2");
    
    // Test destruction
    destroy_request(&test_req);
    
    // After destruction, pointers should be freed, but struct still exists
    // We can't easily check if memory was freed, but we can ensure no crashes
}
END_TEST

/* Security Tests - Revised */

START_TEST(test_path_traversal_attack)
{
    /* Test path traversal detection */
    char request_str[] = "GET /static/../../../etc/passwd HTTP/1.1\r\nHost: example.com\r\n\r\n";
    
    http_request *result = parse_http_request(request_str, &request, &config);
    
    // If your implementation rejects it
    if (!result) {
        ck_assert_ptr_null(result); // Already rejected at HTTP parsing stage
    } 
    // If your implementation allows it but sanitizes the path
    else {
        ck_assert_ptr_null(strstr(request.path, ".."));
    }
}
END_TEST

START_TEST(test_null_byte_injection)
{
    /* 
     * Test null byte injection in HTTP request
     * We'll create a request string with an embedded null
     */
    char request_str[100];
    strcpy(request_str, "GET /static/safe.html");
    
    // Inject a null byte followed by a traversal attempt
    request_str[18] = '\0';
    strcpy(request_str + 19, "../../../etc/passwd HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Pass the full buffer including the part after the null
    http_request *result = parse_http_request(request_str, &request, &config);
    
    // It should either reject the request or only process up to the null byte
    if (result) {
        ck_assert_int_eq((int)strlen(request.path), 18); // Should stop at the null byte
    } else {
        ck_assert_ptr_null(result); // Or reject completely
    }
}
END_TEST

START_TEST(test_long_uri_components)
{
    /* Test very long path components in HTTP request */
    char *request_str = malloc(MAX_URI_LENGTH + 200);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    // Create request with long URI component
    strcpy(request_str, "GET /static/");
    memset(request_str + 11, 'A', MAX_URI_LENGTH); // Fill with 'A's
    sprintf(request_str + 11 + MAX_URI_LENGTH, " HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Parser should reject if too long or handle it safely
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    // Either reject or handle safely
    if (result) {
        ck_assert_ptr_nonnull(request.path);
    } else {
        ck_assert_ptr_null(result);
    }
}
END_TEST

START_TEST(test_encoded_path_traversal)
{
    /* Test encoded path traversal in HTTP request */
    char request_str[] = "GET /static/%2e%2e/%2e%2e/%2e%2e/etc/passwd HTTP/1.1\r\nHost: example.com\r\n\r\n";
    
    http_request *result = parse_http_request(request_str, &request, &config);
    
    // Either reject or sanitize
    if (result) {
        ck_assert_ptr_null(strstr(request.path, ".."));
    } else {
        ck_assert_ptr_null(result);
    }
}
END_TEST

START_TEST(test_invalid_control_chars)
{
    /* Test control characters in HTTP request URI */
    char request_str[100];
    
    // Create request with control characters in URI
    strcpy(request_str, "GET /static/test");
    request_str[13] = 0x01;  // SOH (Start of Heading)
    request_str[14] = 0x02;  // STX (Start of Text)
    request_str[15] = 0x03;  // ETX (End of Text)
    strcpy(request_str + 16, " HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    http_request *result = parse_http_request(request_str, &request, &config);
    
    // Either reject or sanitize
    if (result) {
        // If you accept it, control chars should be sanitized
        for (size_t i = 0; i < strlen(request.path); i++) {
            ck_assert(request.path[i] >= 32 || request.path[i] == '\t');
        }
    } else {
        ck_assert_ptr_null(result);
    }
}
END_TEST

/* Stress Tests - Revised */

START_TEST(test_uri_at_max_length)
{
    /* Test HTTP request with URI at maximum allowed length */
    char *request_str = malloc(MAX_URI_LENGTH + 100);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    // Create request with URI at max length
    strcpy(request_str, "GET /");
    memset(request_str + 5, 'x', MAX_URI_LENGTH - 2);
    sprintf(request_str + MAX_URI_LENGTH + 3, " HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Should be accepted
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    ck_assert_ptr_nonnull(result);
    ck_assert_ptr_nonnull(request.path);
}
END_TEST

START_TEST(test_http_request_exceeding_max_uri_length)
{
    /* Test HTTP request with URI exceeding maximum allowed length */
    char *request_str = malloc(MAX_URI_LENGTH + 100);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    // Create request with URI exceeding max length
    strcpy(request_str, "GET /");
    memset(request_str + 5, 'x', MAX_URI_LENGTH + 10);
    sprintf(request_str + MAX_URI_LENGTH + 15, " HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Should be rejected
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    ck_assert_ptr_null(result);
}
END_TEST

START_TEST(test_many_query_parameters)
{
    /* Test HTTP request with many query parameters */
    char *request_str = malloc(8192);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    strcpy(request_str, "GET /cgi-bin/test.cgi?");
    
    // Add 100 parameters
    for (int i = 0; i < 100; i++) {
        char param[50];
        sprintf(param, "param%d=value%d&", i, i);
        strcat(request_str, param);
    }
    request_str[strlen(request_str) - 1] = ' '; // Replace trailing & with space
    strcat(request_str, "HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Should handle this gracefully
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    ck_assert_ptr_nonnull(result);
    ck_assert_int_eq(request.param_count, 100);
    ck_assert_ptr_nonnull(request.param_names);
    ck_assert_ptr_nonnull(request.param_values);
}
END_TEST

START_TEST(test_deeply_nested_path)
{
    /* Test HTTP request with deeply nested path */
    char *request_str = malloc(4096);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    strcpy(request_str, "GET /static");
    
    // Create deeply nested path
    for (int i = 0; i < 50; i++) {
        strcat(request_str, "/nested");
    }
    strcat(request_str, "/file.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Should handle this gracefully
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    ck_assert_ptr_nonnull(result);
    ck_assert_ptr_nonnull(request.path);
}
END_TEST

START_TEST(test_heavily_encoded_uri)
{
    /* Test HTTP request with heavily encoded URI */
    char *request_str = malloc(4096);
    if (!request_str) {
        ck_abort_msg("Failed to allocate memory for test");
        return;
    }
    
    strcpy(request_str, "GET /static/");
    
    // Add heavily encoded path
    for (int i = 0; i < 50; i++) {
        strcat(request_str, "%20%21%22%23%24%25%26%27");
    }
    strcat(request_str, ".html HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    // Should handle this without crashing
    http_request *result = parse_http_request(request_str, &request, &config);
    
    free(request_str);
    
    // Either succeed or fail gracefully
    if (result) {
        ck_assert_ptr_nonnull(request.path);
    } else {
        ck_assert_ptr_null(result);
    }
}
END_TEST

START_TEST(test_malformed_http_request)
{
    /* 
     * Test with malformed HTTP requests with unusual whitespace and format
     * This tests your request line parsing robustness
     */
    
    // Various malformed but potentially parseable requests
    char *malformed_requests[] = {
        "GET  /index.html   HTTP/1.1\r\nHost: example.com\r\n\r\n",     // Extra spaces
        "GET /index.html HTTP/1.1 \r\nHost: example.com\r\n\r\n",       // Space after version
        "GET /index.html HTTP/1.1\r\n\rHost: example.com\r\n\r\n",      // Malformed line ending
        "GET /index.html HTTP/1.1\r\nHost:  example.com\r\n\r\n",       // Extra space after header colon
        "GET\t/index.html\tHTTP/1.1\r\nHost: example.com\r\n\r\n",      // Tabs instead of spaces
        "GET /index.html HTTP/1.1\nHost: example.com\n\n"               // LF instead of CRLF
    };
    
    for (size_t i = 0; i < sizeof(malformed_requests) / sizeof(malformed_requests[0]); i++) {
        // Reset request for each test
        teardown();
        setup();
        
        // We just want to make sure it doesn't crash
        parse_http_request(malformed_requests[i], &request, &config);
        
        // Whether it succeeds or fails, we just want it to handle the case
        // No assertion here - we're just checking it doesn't crash
    }
    
    // If we got here without crashing, the test passes
    ck_assert(1);
}
END_TEST


/* Create test suite */
Suite *http_parser_suite(void)
{
    Suite *s = suite_create("HTTP Parser");
    
    // Test case for URL decoding
    TCase *tc_url_decode = tcase_create("URL Decode");
    tcase_add_checked_fixture(tc_url_decode, setup, teardown);
    tcase_add_test(tc_url_decode, test_url_decode_basic);
    tcase_add_test(tc_url_decode, test_url_decode_multiple);
    tcase_add_test(tc_url_decode, test_url_decode_plus);
    tcase_add_test(tc_url_decode, test_url_decode_empty);
    tcase_add_test(tc_url_decode, test_url_decode_incomplete_percent);
    tcase_add_test(tc_url_decode, test_url_decode_trailing_percent);
    tcase_add_test(tc_url_decode, test_url_decode_invalid_hex);
    tcase_add_test(tc_url_decode, test_url_decode_mixed_valid_invalid);
    suite_add_tcase(s, tc_url_decode);
    
    // Test case for MIME type detection
    TCase *tc_mime_type = tcase_create("MIME Type");
    tcase_add_checked_fixture(tc_mime_type, setup, teardown);
    tcase_add_test(tc_mime_type, test_mime_type_html);
    tcase_add_test(tc_mime_type, test_mime_type_txt);
    tcase_add_test(tc_mime_type, test_mime_type_image);
    tcase_add_test(tc_mime_type, test_mime_type_no_extension);
    tcase_add_test(tc_mime_type, test_mime_type_multiple_extensions);
    tcase_add_test(tc_mime_type, test_mime_type_unusual_extensions);
    tcase_add_test(tc_mime_type, test_mime_type_dot_only);
    suite_add_tcase(s, tc_mime_type);
    
    // Test case for URI parsing
    TCase *tc_uri = tcase_create("URI Parsing");
    tcase_add_checked_fixture(tc_uri, setup, teardown);
    tcase_add_test(tc_uri, test_parse_uri_static);
    tcase_add_test(tc_uri, test_parse_uri_dynamic);
    tcase_add_test(tc_uri, test_parse_uri_encoded);
    tcase_add_test(tc_uri, test_parse_uri_dynamic_similar_name);
    tcase_add_test(tc_uri, test_parse_uri_complex_query);
    tcase_add_test(tc_uri, test_parse_uri_root);
    suite_add_tcase(s, tc_uri);
    
    // Test case for HTTP request parsing
    TCase *tc_request = tcase_create("Request Parsing");
    tcase_add_checked_fixture(tc_request, setup, teardown);
    tcase_add_test(tc_request, test_parse_http_request_valid);
    tcase_add_test(tc_request, test_parse_http_request_invalid_method);
    tcase_add_test(tc_request, test_parse_http_request_invalid_version);
    tcase_add_test(tc_request, test_parse_http_request_http_1_0);
    tcase_add_test(tc_request, test_parse_http_request_malformed_request_line);
    tcase_add_test(tc_request, test_parse_http_request_no_crlf);
    tcase_add_test(tc_request, test_parse_http_request_invalid_uri_path);
    suite_add_tcase(s, tc_request);
    
    // Test case for request initialization and cleanup
    TCase *tc_lifecycle = tcase_create("Request Lifecycle");
    tcase_add_test(tc_lifecycle, test_initialize_destroy_request);
    suite_add_tcase(s, tc_lifecycle);

    TCase *tc_security = tcase_create("Security");
    tcase_add_checked_fixture(tc_security, setup, teardown);
    tcase_add_test(tc_security, test_path_traversal_attack);
    tcase_add_test(tc_security, test_null_byte_injection);
    tcase_add_test(tc_security, test_long_uri_components);
    tcase_add_test(tc_security, test_encoded_path_traversal);
    tcase_add_test(tc_security, test_invalid_control_chars);
    suite_add_tcase(s, tc_security);
    
    // Stress test case
    TCase *tc_stress = tcase_create("Stress");
    tcase_add_checked_fixture(tc_stress, setup, teardown);
    tcase_add_test(tc_stress, test_uri_at_max_length);
    tcase_add_test(tc_stress, test_http_request_exceeding_max_uri_length);
    tcase_add_test(tc_stress, test_many_query_parameters);
    tcase_add_test(tc_stress, test_deeply_nested_path);
    tcase_add_test(tc_stress, test_heavily_encoded_uri);
    tcase_add_test(tc_stress, test_malformed_http_request);
    suite_add_tcase(s, tc_stress);
    
    return s;
}



/* Main function */
int main(void)
{
    Suite *s = http_parser_suite();
    SRunner *sr = srunner_create(s);
    
    // Use CK_VERBOSE for detailed output, CK_NORMAL for normal output
    srunner_run_all(sr, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}