#ifndef HUFFMAN_ENCODED_CHARACTER
#define HUFFMAN_ENCODED_CHARACTER

#include <string>
#include <vector>
#include <bitset>

#include "../definitions.h"
#include "../utils.h"

namespace huffman::encoder
{
    struct encodedCharacter {
        byte bits;
        byte code[TABLE_SIZE_BYTES];

        encodedCharacter();
        encodedCharacter(std::vector<byte>::const_iterator& serialized);
        encodedCharacter(std::vector<byte>::const_iterator&& serialized);

        void append_bit(bool bit);
        bool get_bit(byte bit) const;

        inline byte bytes() const {
            return positive_div_ceil<byte>(bits, 8);
        }

        inline byte last_byte_bits() const {
            return ((bits - 1) % 8) + 1;
        }

        std::string to_string() const;
        std::vector<byte> serialize() const;

        bool is_prefix(const encodedCharacter& other) const;

        friend bool operator==(const encodedCharacter& lhs, const encodedCharacter& rhs);
    };
}

#endif