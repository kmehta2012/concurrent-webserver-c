#!/usr/bin/env python3
"""
Comprehensive Test Suite for HTTP Server Stage 2

Tests all files, headers, content, CGI functionality, and error handling.
Creates temporary test files as needed and cleans them up.
"""

import requests
import os
from typing import Dict, List, Tuple, Optional

# Server configuration
SERVER_BASE_URL = "http://localhost:8080"
PUBLIC_DIR = "./public"

# Colors for output
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'

class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
    
    def add_pass(self, test_name: str):
        self.passed += 1
        print(f"{Colors.GREEN}✓{Colors.END} {test_name}")
    
    def add_fail(self, test_name: str, error: str):
        self.failed += 1
        self.errors.append(f"{test_name}: {error}")
        print(f"{Colors.RED}✗{Colors.END} {test_name}: {Colors.RED}{error}{Colors.END}")
    
    def summary(self):
        total = self.passed + self.failed
        print(f"\n{Colors.BOLD}Test Summary:{Colors.END}")
        print(f"Total tests: {total}")
        print(f"{Colors.GREEN}Passed: {self.passed}{Colors.END}")
        print(f"{Colors.RED}Failed: {self.failed}{Colors.END}")
        if self.failed > 0:
            print(f"\n{Colors.RED}Failures:{Colors.END}")
            for error in self.errors:
                print(f"  {error}")
        return self.failed == 0

class HTTPServerTester:
    def __init__(self):
        self.result = TestResult()
        self.temp_files = []  # Track temporary files for cleanup
        
    def cleanup(self):
        """Remove all temporary test files"""
        for file_path in self.temp_files:
            try:
                if os.path.exists(file_path):
                    os.remove(file_path)
                    print(f"Cleaned up: {file_path}")
            except Exception as e:
                print(f"Warning: Could not remove {file_path}: {e}")
    
    def create_temp_file(self, relative_path: str, content, binary: bool = False) -> str:
        """Create a temporary test file"""
        full_path = os.path.join(PUBLIC_DIR, relative_path.lstrip('/'))
        os.makedirs(os.path.dirname(full_path), exist_ok=True)
        
        mode = 'wb' if binary else 'w'
        with open(full_path, mode) as f:
            if binary:
                f.write(content)  # content should already be bytes
            else:
                f.write(content)
        
        self.temp_files.append(full_path)
        return full_path
    
    def read_file_content(self, file_path: str) -> bytes:
        """Read file content as bytes"""
        try:
            with open(file_path, 'rb') as f:
                return f.read()
        except FileNotFoundError:
            return b""
    
    def test_file(self, url_path: str, file_path: str, expected_content_type: str, test_name: str = None):
        """Test a single file for correct headers and content"""
        if test_name is None:
            test_name = f"File: {url_path}"
        
        try:
            # Make request
            response = requests.get(f"{SERVER_BASE_URL}{url_path}")
            
            # Check status code
            if response.status_code != 200:
                self.result.add_fail(test_name, f"Expected 200, got {response.status_code}")
                return
            
            # Read expected content
            expected_content = self.read_file_content(file_path)
            actual_content = response.content
            
            # Check content
            if actual_content != expected_content:
                self.result.add_fail(test_name, f"Content mismatch. Expected {len(expected_content)} bytes, got {len(actual_content)} bytes")
                return
            
            # Check Content-Type
            content_type = response.headers.get('Content-Type', '')
            if content_type != expected_content_type:
                self.result.add_fail(test_name, f"Wrong Content-Type. Expected '{expected_content_type}', got '{content_type}'")
                return
            
            # Check Content-Length
            expected_length = len(expected_content)
            content_length = response.headers.get('Content-Length')
            if content_length != str(expected_length):
                self.result.add_fail(test_name, f"Wrong Content-Length. Expected {expected_length}, got {content_length}")
                return
            
            # Check required headers
            required_headers = ['Server', 'Date', 'Connection']
            for header in required_headers:
                if header not in response.headers:
                    self.result.add_fail(test_name, f"Missing required header: {header}")
                    return
            
            # Check Last-Modified (should be present for static files)
            if 'Last-Modified' not in response.headers:
                self.result.add_fail(test_name, "Missing Last-Modified header")
                return
            
            self.result.add_pass(test_name)
            
        except requests.exceptions.RequestException as e:
            self.result.add_fail(test_name, f"Request failed: {e}")
        except Exception as e:
            self.result.add_fail(test_name, f"Test error: {e}")
    
    def test_html_files(self):
        """Test HTML files"""
        print(f"\n{Colors.BLUE}Testing HTML Files{Colors.END}")
        
        html_files = [
            ("/static/html/index.html", "static/html/index.html"),
            ("/static/html/features.html", "static/html/features.html"),
            ("/static/html/special_chars.html", "static/html/special_chars.html"),
        ]
        
        for url_path, file_path in html_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, "text/html")
    
    def test_css_files(self):
        """Test CSS files"""
        print(f"\n{Colors.BLUE}Testing CSS Files{Colors.END}")
        
        css_files = [
            ("/static/css/styles.css", "static/css/styles.css"),
            ("/static/css/layout.css", "static/css/layout.css"),
        ]
        
        for url_path, file_path in css_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, "text/css")
    
    def test_javascript_files(self):
        """Test JavaScript files"""
        print(f"\n{Colors.BLUE}Testing JavaScript Files{Colors.END}")
        
        js_files = [
            ("/static/js/script.js", "static/js/script.js"),
            ("/static/js/interactive.js", "static/js/interactive.js"),
        ]
        
        for url_path, file_path in js_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, "application/javascript")
    
    def test_text_files(self):
        """Test various text files"""
        print(f"\n{Colors.BLUE}Testing Text Files{Colors.END}")
        
        text_files = [
            ("/static/text/readme.txt", "static/text/readme.txt", "text/plain"),
            ("/static/text/sample.json", "static/text/sample.json", "application/json"),
            ("/static/text/sample.xml", "static/text/sample.xml", "application/xml"),
            ("/static/text/atleast_10Kb_file.txt", "static/text/atleast_10Kb_file.txt", "text/plain"),
        ]
        
        for url_path, file_path, content_type in text_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, content_type)
    
    def test_special_filename(self):
        """Test file with special characters"""
        print(f"\n{Colors.BLUE}Testing Special Filenames{Colors.END}")
        
        # URL encode the special characters
        special_file = "/static/text/file%20with%20spaces%20%26%20symbols%20%23%40%21.txt"
        file_path = os.path.join(PUBLIC_DIR, "static/text/file with spaces & symbols #@!.txt")
        self.test_file(special_file, file_path, "text/plain", "Special filename with spaces & symbols")
        
        # Test long filename
        long_file = "/static/text/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt"
        file_path = os.path.join(PUBLIC_DIR, "static/text/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt")
        self.test_file(long_file, file_path, "text/plain", "Very long filename")
    
    def test_media_files(self):
        """Test media files (dummy files with text content)"""
        print(f"\n{Colors.BLUE}Testing Media Files{Colors.END}")
        
        media_files = [
            ("/static/media/dummy.png", "static/media/dummy.png", "image/png"),
            ("/static/media/dummy.jpg", "static/media/dummy.jpg", "image/jpeg"),
            ("/static/media/dummy.gif", "static/media/dummy.gif", "image/gif"),
            ("/static/media/dummy.svg", "static/media/dummy.svg", "image/svg+xml"),
            ("/static/media/dummy.mp3", "static/media/dummy.mp3", "audio/mpeg"),
            ("/static/media/dummy.wav", "static/media/dummy.wav", "audio/wav"),
            ("/static/media/dummy.mp4", "static/media/dummy.mp4", "video/mp4"),
            ("/static/media/dummy.webm", "static/media/dummy.webm", "video/webm"),
        ]
        
        for url_path, file_path, content_type in media_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, content_type)
    
    def test_misc_files(self):
        """Test miscellaneous file types"""
        print(f"\n{Colors.BLUE}Testing Miscellaneous Files{Colors.END}")
        
        misc_files = [
            ("/static/misc/dummy.pdf", "static/misc/dummy.pdf", "application/pdf"),
            ("/static/misc/dummy.zip", "static/misc/dummy.zip", "application/zip"),
            ("/static/misc/sample.ps", "static/misc/sample.ps", "application/postscript"),  # Fixed: was dummy.ps
            ("/static/misc/achive.tar.gz", "static/misc/achive.tar.gz", "application/octet-stream"),  # Updated: now defaults to octet-stream
            ("/static/misc/dummy.woff", "static/misc/dummy.woff", "font/woff"),
            ("/static/misc/dummy.woff2", "static/misc/dummy.woff2", "font/woff2"),
            ("/static/misc/no_extension", "static/misc/no_extension", "application/octet-stream"),  # Updated: no extension = octet-stream
            ("/static/misc/noextension", "static/misc/noextension", "application/octet-stream"),   # Updated: no extension = octet-stream
        ]
        
        for url_path, file_path, content_type in misc_files:
            full_path = os.path.join(PUBLIC_DIR, file_path)
            self.test_file(url_path, full_path, content_type)
    
    def test_cgi_basic(self):
        """Test basic CGI functionality"""
        print(f"\n{Colors.BLUE}Testing CGI Scripts{Colors.END}")
        
        try:
            # Test hello.cgi
            response = requests.get(f"{SERVER_BASE_URL}/cgi-bin/hello.cgi")
            
            if response.status_code != 200:
                self.result.add_fail("CGI hello.cgi", f"Expected 200, got {response.status_code}")
                return
            
            # Check that it contains expected CGI output
            content = response.text
            if "Hello from CGI!" not in content:
                self.result.add_fail("CGI hello.cgi", "Missing expected 'Hello from CGI!' in output")
                return
            
            # Check Content-Type header is present
            if 'Content-Type' not in response.headers:
                self.result.add_fail("CGI hello.cgi", "Missing Content-Type header")
                return
            
            self.result.add_pass("CGI hello.cgi")
            
        except Exception as e:
            self.result.add_fail("CGI hello.cgi", f"Request failed: {e}")
    
    def test_cgi_with_params(self):
        """Test CGI with query parameters"""
        try:
            # Test params.cgi (just checks headers and fixed output)
            response = requests.get(f"{SERVER_BASE_URL}/cgi-bin/params.cgi?name=John&age=25")
            
            if response.status_code != 200:
                self.result.add_fail("CGI params.cgi", f"Expected 200, got {response.status_code}")
                return
            
            # Check for expected content and headers
            if 'X-Test-Header' not in response.headers:
                self.result.add_fail("CGI params.cgi", "Missing custom X-Test-Header")
                return
            
            if response.headers['X-Test-Header'] != 'Value123':
                self.result.add_fail("CGI params.cgi", f"Wrong X-Test-Header value: {response.headers['X-Test-Header']}")
                return
                
            if "Header test completed." not in response.text:
                self.result.add_fail("CGI params.cgi", "Missing expected output text")
                return
            
            self.result.add_pass("CGI params.cgi (headers)")
            
        except Exception as e:
            self.result.add_fail("CGI params.cgi", f"Request failed: {e}")
        
        # Test params_test.cgi (actually processes and shows parameters)
        try:
            response = requests.get(f"{SERVER_BASE_URL}/cgi-bin/params_test.cgi?name=John&age=25&city=Austin")
            
            if response.status_code != 200:
                self.result.add_fail("CGI params_test.cgi", f"Expected 200, got {response.status_code}")
                return
            
            # Check that query string is shown in output
            if "QUERY_STRING=name=John&age=25&city=Austin" not in response.text:
                self.result.add_fail("CGI params_test.cgi", "Query string not found in output")
                return
            
            if "REQUEST_METHOD=GET" not in response.text:
                self.result.add_fail("CGI params_test.cgi", "REQUEST_METHOD not found in output")
                return
            
            self.result.add_pass("CGI params_test.cgi (parameters)")
            
        except Exception as e:
            self.result.add_fail("CGI params_test.cgi", f"Request failed: {e}")
    
    def test_error_responses(self):
        """Test error response handling"""
        print(f"\n{Colors.BLUE}Testing Error Responses{Colors.END}")
        
        # Test 404 for non-existent file
        try:
            response = requests.get(f"{SERVER_BASE_URL}/static/nonexistent.txt")
            if response.status_code != 404:
                self.result.add_fail("404 Error", f"Expected 404, got {response.status_code}")
            else:
                self.result.add_pass("404 Error for non-existent file")
        except Exception as e:
            self.result.add_fail("404 Error", f"Request failed: {e}")
        
        # Test 404 for non-existent CGI
        try:
            response = requests.get(f"{SERVER_BASE_URL}/cgi-bin/nonexistent.cgi")
            if response.status_code != 404:
                self.result.add_fail("404 CGI Error", f"Expected 404, got {response.status_code}")
            else:
                self.result.add_pass("404 Error for non-existent CGI")
        except Exception as e:
            self.result.add_fail("404 CGI Error", f"Request failed: {e}")
    
    def test_security(self):
        """Test security-related requests"""
        print(f"\n{Colors.BLUE}Testing Security & Malformed Requests{Colors.END}")
        
        # Test path traversal attempts
        traversal_attempts = [
            "/static/../../../etc/passwd",
            "/static/%2e%2e/%2e%2e/%2e%2e/etc/passwd",
            "/static/..\\..\\..\\windows\\system32\\config\\sam",
        ]
        
        for attempt in traversal_attempts:
            try:
                response = requests.get(f"{SERVER_BASE_URL}{attempt}")
                # Should get 400, 403, or 404 - not 200
                if response.status_code == 200:
                    self.result.add_fail(f"Path traversal: {attempt}", f"Security issue: got 200, should reject")
                else:
                    self.result.add_pass(f"Path traversal protection: {attempt}")
            except Exception as e:
                self.result.add_pass(f"Path traversal protection: {attempt} (connection rejected)")
    
    def test_edge_cases(self):
        """Test edge cases and unusual requests"""
        print(f"\n{Colors.BLUE}Testing Edge Cases{Colors.END}")
        
        # Test root directory
        try:
            response = requests.get(f"{SERVER_BASE_URL}/")
            # Should either serve directory listing or index file, not crash
            if response.status_code in [200, 403, 404]:
                self.result.add_pass("Root directory request")
            else:
                self.result.add_fail("Root directory", f"Unexpected status: {response.status_code}")
        except Exception as e:
            self.result.add_fail("Root directory", f"Request failed: {e}")
        
        # Test very long URL (but not too long to cause issues)
        long_path = "/static/" + "a" * 100 + ".txt"
        try:
            response = requests.get(f"{SERVER_BASE_URL}{long_path}")
            # Should get 404 (file doesn't exist) not crash
            if response.status_code == 404:
                self.result.add_pass("Long URL handling")
            else:
                self.result.add_fail("Long URL", f"Expected 404, got {response.status_code}")
        except Exception as e:
            self.result.add_fail("Long URL", f"Request failed: {e}")
    
    def create_missing_test_files(self):
        """Create any missing test files needed for comprehensive testing"""
        print(f"\n{Colors.YELLOW}Creating temporary test files...{Colors.END}")
        
        # Create a binary test file (small, simple)
        binary_content = b'\x00\x01\x02\x03\xFF\xFE\xFD\x80\x81\x82'
        binary_path = self.create_temp_file("static/misc/test_binary.bin", binary_content, binary=True)
        print(f"Created: {binary_path}")
        
        # Create additional CGI test file
        cgi_content = '''#!/bin/bash
echo "Content-Type: text/plain"
echo "X-Custom-Header: TestValue"
echo ""
echo "Additional CGI Test"
echo "SERVER_NAME=$SERVER_NAME"
echo "SCRIPT_NAME=$SCRIPT_NAME"
'''
        cgi_path = self.create_temp_file("cgi-bin/test_env.cgi", cgi_content)
        os.chmod(cgi_path, 0o755)  # Make executable
        print(f"Created: {cgi_path}")
    
    def test_additional_files(self):
        """Test the temporary files we created"""
        print(f"\n{Colors.BLUE}Testing Additional Created Files{Colors.END}")
        
        # Test the binary file we created
        binary_path = os.path.join(PUBLIC_DIR, "static/misc/test_binary.bin")
        self.test_file("/static/misc/test_binary.bin", binary_path, "application/octet-stream", "Binary file test")
        
        # Test additional CGI
        try:
            response = requests.get(f"{SERVER_BASE_URL}/cgi-bin/test_env.cgi")
            if response.status_code == 200 and "Additional CGI Test" in response.text:
                self.result.add_pass("Additional CGI test")
            else:
                self.result.add_fail("Additional CGI test", f"Status: {response.status_code}, missing expected content")
        except Exception as e:
            self.result.add_fail("Additional CGI test", f"Request failed: {e}")
    
    def run_all_tests(self):
        """Run the complete test suite"""
        print(f"{Colors.BOLD}HTTP Server Stage 2 Test Suite{Colors.END}")
        print(f"Testing server at: {SERVER_BASE_URL}")
        print("=" * 50)
        
        # Create any missing test files
        self.create_missing_test_files()
        
        try:
            # Test server is running
            try:
                response = requests.get(SERVER_BASE_URL, timeout=5)
                print(f"{Colors.GREEN}✓{Colors.END} Server is responding")
            except requests.exceptions.RequestException:
                print(f"{Colors.RED}✗{Colors.END} Server is not responding at {SERVER_BASE_URL}")
                print("Make sure your server is running on port 8080")
                return False
            
            # Run all test categories
            self.test_html_files()
            self.test_css_files()
            self.test_javascript_files()
            self.test_text_files()
            self.test_special_filename()
            self.test_media_files()
            self.test_misc_files()
            self.test_cgi_basic()
            self.test_cgi_with_params()
            self.test_additional_files()
            self.test_error_responses()
            self.test_security()
            self.test_edge_cases()
            
            # Print summary
            success = self.result.summary()
            return success
            
        finally:
            # Always cleanup temporary files
            print(f"\n{Colors.YELLOW}Cleaning up temporary files...{Colors.END}")
            self.cleanup()

def main():
    """Main function"""
    tester = HTTPServerTester()
    try:
        success = tester.run_all_tests()
        exit_code = 0 if success else 1
        exit(exit_code)
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Test interrupted by user{Colors.END}")
        tester.cleanup()
        exit(1)
    except Exception as e:
        print(f"\n{Colors.RED}Test suite error: {e}{Colors.END}")
        tester.cleanup()
        exit(1)

if __name__ == "__main__":
    main()