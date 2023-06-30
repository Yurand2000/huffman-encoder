#ifndef TEST_UTILS
#define TEST_UTILS

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "[unknown source file]"
#endif

#include <iostream>

template<typename... Args>
void assert(bool expression, Args&&... args) {
    if (!expression) {
        (std::cout << ... << std::forward<Args>(args));
        std::cout << std::endl << std::flush;
        std::cout << TEST_FILE_NAME << " - Failed" << std::endl;
        std::abort();
    }
}

void testMain();
int main()
{
    std::cout << TEST_FILE_NAME << " - Start" << std::endl;

    testMain();

    std::cout << TEST_FILE_NAME << " - Pass" << std::endl;
}

#endif