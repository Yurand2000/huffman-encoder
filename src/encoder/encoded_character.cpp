#include "encoded_character.h"

#include "../utils.h"

namespace huffman::encoder::detail
{
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
}

namespace huffman::encoder
{
    encodedCharacter::encodedCharacter() : bits(0) {
        for(size_t i = 0; i < TABLE_SIZE_BYTES; i++)
            code[i] = 0;
    }

    encodedCharacter::encodedCharacter(std::vector<byte>::const_iterator& serialized)
        : encodedCharacter()
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-overflow"

        bits = *serialized;

        auto curr_byte = *(++serialized);
        for (size_t i = 0; i < bytes() - 1; i++) {
            code[i] = curr_byte;
            curr_byte = *(++serialized);
        }

        code[ bytes() - 1 ] = curr_byte & detail::leftByteMasks[last_byte_bits()];
        curr_byte = *(++serialized);

        #pragma GCC diagnostic pop
    }
    
    encodedCharacter::encodedCharacter(std::vector<byte>::const_iterator&& serialized)
        : encodedCharacter(serialized) {}

    void encodedCharacter::append_bit(bool bit) {
        bits += 1;
        code[bytes() - 1] |= bit << (8 - last_byte_bits());
    }
    
    bool encodedCharacter::get_bit(byte bit) const {
        return (code[positive_div_ceil<byte>(bit, 8) - 1] & (detail::leftByteMasks[1] >> (bit - 1) % 8)) > 0;
    }

    std::string encodedCharacter::to_string() const {
        auto string = std::string();
        string += "[" + std::to_string(bits) + " bits, code: ";

        for (size_t i = 0; i < bytes() - 1; i++) {
            for (size_t j = 8; j > 0; j--) {
                auto bit = ( code[i] & (1 << (j-1)) ) > 0;
                string += bit ? "1" : "0";
            }
            string += " ";
        }
        
        for (size_t j = 8; j > 8 - last_byte_bits(); j--) {
            auto bit = ( code[bytes() - 1] & (1 << (j-1)) ) > 0;
            string += bit ? "1" : "0";
        }

        string += "]";

        return string;
    }

    std::vector<byte> encodedCharacter::serialize() const {
        auto serialized = std::vector<byte>();
        serialized.reserve(1 + bytes());
        serialized.push_back(bits);

        for (size_t i = 0; i < bytes(); i++) {
            serialized.push_back( code[ i ] );
        }

        return serialized;
    }

    bool encodedCharacter::is_prefix(const encodedCharacter& other) const
    {
        if (bits > other.bits) {
            return false;
        } else {
            for(int i = 0; i < bytes() - 1; i++) {
                if (code[i] != other.code[i])
                    return false;
            }

            auto last_byte_mask = detail::leftByteMasks[last_byte_bits()];
            return (code[bytes() - 1] & last_byte_mask) == (other.code[bytes() - 1] & last_byte_mask);
        }
    }

    bool operator==(const encodedCharacter& lhs, const encodedCharacter& rhs) {
        if (lhs.bits != rhs.bits)
            return false;
        else {
            for(size_t i = 0; i < lhs.bytes(); i++) {
                if (lhs.code[i] != rhs.code[i])
                    return false;
            }
            return true;
        }
    }
}