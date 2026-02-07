#include "test_primitive/test_primitive_id_mgr.h"
#include "timer.h"
#include <iostream>
#include <vector>

int main() {
    std::vector<bool(*)()> test_functions = {
            test_id_funcs,
    };
    unsigned int tests_passed = 0;
    for (int i = 0; i < test_functions.size(); i++) {
        std::cout << "Running test [" << i << "]: ";
        // Timer::start();
        bool result = test_functions[i]();
        // Timer::end(4, "");
        if (result) {
            std::cout << "Passed" << std::endl;
            tests_passed++;
        }
        else { std::cout << "Failed" << std::endl; }
    }
    std::cout << "---------------------------" << std::endl;
    std::cout << "Tests passed: " << tests_passed << "/" << test_functions.size() << std::endl;
    std::cout << "Tests failed: " << test_functions.size()-tests_passed << "/" << test_functions.size() << std::endl;

    return 0;
};
