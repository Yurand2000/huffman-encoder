#include "encoded_character.h"

#include "../utils.h"

namespace huffman::encoder
{
    encodedCharacter::encodedCharacter() : bits(0), code(0) {}

    encodedCharacter::encodedCharacter(std::vector<byte>::const_iterator& bytes) {
        bits = *bytes;

        auto number_of_bytes = positive_div_ceil<byte>(bits, 8);

        auto curr_byte = *(++bytes);
        for (size_t i = number_of_bytes; i > 0; i--) {
            for (size_t j = 0; j < 8; j++)
                code[ (i-1)*8 + j ] = (curr_byte & (1 << j)) >> j;
            curr_byte = *(++bytes);
        }
    }
    
    encodedCharacter::encodedCharacter(std::vector<byte>::const_iterator&& bytes)
        : encodedCharacter(bytes) {}

    void encodedCharacter::append_bit(bool bit) {
        bits += 1;
        code <<= 1;
        code.set(0, bit);
    }

    std::vector<bool> encodedCharacter::get() const {
        if (bits == 0) return {};

        auto vector = std::vector<bool>();
        vector.reserve(bits);
        for (size_t i = bits; i > 0; i--) {
            vector.push_back( code[i-1] );
        }

        return vector;
    }

    std::string encodedCharacter::to_string() const {
        auto string = std::string();
        for (size_t i = bits; i > 0; i--) {
            string.push_back(code[i-1] ? '1' : '0');
        }

        return string;
    }

    std::vector<byte> encodedCharacter::serialize() const {
        auto bytes = positive_div_ceil<byte>(bits, 8);

        auto serialized = std::vector<byte>();
        serialized.reserve(1 + bytes);
        serialized.push_back(bits);

        for (size_t i = bytes; i > 0; i--) {
            byte code = 0;
            for (size_t j = 0; j < 8; j++)
                code |= this->code[ (i-1)*8 + j ] << j;
            serialized.push_back(code);
        }

        return serialized;
    }

    bool operator==(const encodedCharacter& lhs, const encodedCharacter& rhs) {
        if (lhs.bits != rhs.bits)
            return false;
        else {
            for(size_t i = 0; i < lhs.bits; i++) {
                if (lhs.code[i] != rhs.code[i])
                    return false;
            }
            return true;
        }
    }
}