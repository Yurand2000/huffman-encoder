#ifndef BIT_STREAM
#define BIT_STREAM

#include <vector>
#include "definitions.h"

namespace huffman
{
    struct bitStream {
        const std::vector<byte>& data;
        size_t last_byte, last_bit;

        bitStream(const std::vector<byte>& data);

        void advance(size_t bits);
        bool hasNext() const;

        bool operator*() const;
        bitStream& operator++();
    };
}

#endif