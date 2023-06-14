#ifndef UTILS
#define UTILS

#include <iostream>

template<typename T>
inline T positive_div_ceil(T dividend, T divisor) {
    if (dividend == 0) {
        return 0;
    } else {
        return 1 + ((dividend - 1) / divisor);
    }
}

template<typename... Args>
void assert_fn(bool expression, const char* filename, Args&&... args) {
    if (!expression) {
        std::cerr << "Test Error in file: " << filename << std::endl;
        (std::cerr << ... << std::forward<Args>(args));
        std::cerr << std::endl << std::flush;
        std::abort();
    }
}

#endif