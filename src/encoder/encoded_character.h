#ifndef HUFFMAN_ENCODED_CHARACTER
#define HUFFMAN_ENCODED_CHARACTER

#include <string>
#include <vector>

#include "../definitions.h"
#include "../utils.h"

namespace huffman::encoder
{
    struct encodedCharacter {
    public:
        byte bits;
        byte code[TABLE_SIZE_BYTES];

    public:
        encodedCharacter();
        encodedCharacter(std::vector<byte>::const_iterator& serialized);
        encodedCharacter(std::vector<byte>::const_iterator&& serialized);

        void append_bit(bool bit);
        bool get_bit(byte bit) const;

        inline byte bytes() const {
            return positive_div_ceil<byte>(bits, 8);
        }

        inline byte last_byte_bits() const {
            if (bits == 0) {
                return 0;
            } else {
                return ((bits - 1) % 8) + 1;
            }
        }

        std::string to_string() const;
        std::vector<byte> serialize() const;

        bool is_prefix(const encodedCharacter& other) const;

        inline byte operator[](size_t pos) const {
            return code[pos];
        }

        friend bool operator==(const encodedCharacter& lhs, const encodedCharacter& rhs);
    };
}

#endif