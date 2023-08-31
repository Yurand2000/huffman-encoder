#ifndef TEST_UTILS
#define TEST_UTILS

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "[unknown source file]"
#endif

#include <iostream>
#include <exception>

//unit test utilities
template<typename... Args>
void assert(bool expression, Args&&... args) {
    if (!expression) {
        (std::cout << ... << std::forward<Args>(args));
        std::cout << std::endl << std::flush;
        std::cout << TEST_FILE_NAME << " - Failed" << std::endl;
        std::abort();
    }
}

void print_exception_and_abort(const std::exception& e) {
    std::cout << "Termination with exception: " << e.what() << std::endl << std::flush;
    std::cout << TEST_FILE_NAME << " - Failed" << std::endl;
    std::abort();
}

void testMain();
int main()
{
    std::cout << TEST_FILE_NAME << " - Start" << std::endl;

    try {
        testMain();
    } catch (const std::exception& e) {
        print_exception_and_abort(e);
    }

    std::cout << TEST_FILE_NAME << " - Pass" << std::endl;
    return 0;
}

#endif