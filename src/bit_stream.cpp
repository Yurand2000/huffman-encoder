#include "bit_stream.h"

#include <bitset>

#include "utils.h"

namespace huffman
{
    bitStream::bitStream(const std::vector<byte>& data)
        : data(data), last_byte(0), last_bit(0) {}

    void bitStream::advance(size_t bits) {
        last_byte = positive_div_ceil(bits, static_cast<size_t>(8));
        last_bit = bits % 8; 
    }

    bool bitStream::hasNext() const {
        return last_byte < data.size();
    }

    bool bitStream::operator*() const {
        return std::bitset<8>(data[last_byte])[7 - last_bit];
    }

    bitStream& bitStream::operator++() {
        if (last_bit >= 7) {
            last_byte += 1;
            last_bit = 0;
        } else {
            last_bit += 1;
        }

        return *this;
    }
}