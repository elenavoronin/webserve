#include "../include/CGI.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert> //for assertions

int passed = 0;
int failed = 0;

// Simple test function
void test(const std::string& testName, bool condition) {
    if (condition) {
        //std::cout << testName << ": Passed\n";
        passed++;
    } else {
        std::cerr << testName << ": Failed\n";
        failed++;
    }
}

int main() {
	CGI cgi;

	//Test urlDecode()
	//Test get_env()
	//Test parse_query_string()
	//Summary of test results
	//std::cout << "\n" << passed << " tests passed, " << failed << " tests failed.\n";
    return failed == 0 ? 0 : 1;  // Return 0 if all tests pass, non-zero if any test fails
}