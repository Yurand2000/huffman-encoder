#ifndef UTILS
#define UTILS

template<typename T>
inline T positive_div_ceil(T dividend, T divisor) {
    if (dividend == 0) {
        return 0;
    } else {
        return 1 + ((dividend - 1) / divisor);
    }
}

#endif