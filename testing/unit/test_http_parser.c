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
    suite_add_tcase(s, tc_url_decode);
    
    // Test case for MIME type detection
    TCase *tc_mime_type = tcase_create("MIME Type");
    tcase_add_checked_fixture(tc_mime_type, setup, teardown);
    tcase_add_test(tc_mime_type, test_mime_type_html);
    tcase_add_test(tc_mime_type, test_mime_type_txt);
    tcase_add_test(tc_mime_type, test_mime_type_image);
    tcase_add_test(tc_mime_type, test_mime_type_no_extension);
    suite_add_tcase(s, tc_mime_type);
    
    // Test case for URI parsing
    TCase *tc_uri = tcase_create("URI Parsing");
    tcase_add_checked_fixture(tc_uri, setup, teardown);
    tcase_add_test(tc_uri, test_parse_uri_static);
    tcase_add_test(tc_uri, test_parse_uri_dynamic);
    tcase_add_test(tc_uri, test_parse_uri_encoded);
    suite_add_tcase(s, tc_uri);
    
    // Test case for HTTP request parsing
    TCase *tc_request = tcase_create("Request Parsing");
    tcase_add_checked_fixture(tc_request, setup, teardown);
    tcase_add_test(tc_request, test_parse_http_request_valid);
    tcase_add_test(tc_request, test_parse_http_request_invalid_method);
    tcase_add_test(tc_request, test_parse_http_request_invalid_version);
    suite_add_tcase(s, tc_request);
    
    // Test case for request initialization and cleanup
    TCase *tc_lifecycle = tcase_create("Request Lifecycle");
    tcase_add_test(tc_lifecycle, test_initialize_destroy_request);
    suite_add_tcase(s, tc_lifecycle);
    
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