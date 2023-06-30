#ifndef HUFFMAN_ENCODED_CHARACTER
#define HUFFMAN_ENCODED_CHARACTER

#include <string>
#include <vector>
#include <bitset>

#include "../definitions.h"

namespace huffman::encoder
{
    struct encodedCharacter {
        byte bits;
        std::bitset<TABLE_SIZE> code;

        encodedCharacter();
        encodedCharacter(std::vector<byte>::const_iterator& bytes);
        encodedCharacter(std::vector<byte>::const_iterator&& bytes);

        void append_bit(bool bit);
        std::vector<bool> get() const;

        std::string to_string() const;

        std::vector<byte> serialize() const;

        friend bool operator==(const encodedCharacter& lhs, const encodedCharacter& rhs);
    };
}

#endif