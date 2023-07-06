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

const static byte rightByteMasks[] = {
    0,
    0b00000001,
    0b00000011,
    0b00000111,
    0b00001111,
    0b00011111,
    0b00111111,
    0b01111111,
    0b11111111
};

const static byte leftByteMasks[] = {
    0,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100,
    0b11111110,
    0b11111111,
};

#endif