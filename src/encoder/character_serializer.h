#ifndef HUFFMAN_CHARACTER_SERIALIZER
#define HUFFMAN_CHARACTER_SERIALIZER

#include "encoder_table.h"
#include "../utils.h"

namespace huffman::encoder::detail
{
    using namespace huffman::encoder;

    struct characterSerializer {
    private:
        const encoderTable& table;
        std::vector<byte>& data;
        byte last_bit;

    public:
        characterSerializer(const encoderTable& table, std::vector<byte>& data, byte offset = 8);

        inline void append(char character);
    };

    void characterSerializer::append(char character) {
        auto& encoding = table.get(character);

        if (last_bit == 8) {
            //just append the character encoding, because it is right aligned.
            for(size_t i = 0; i < encoding.bytes(); i++)
                data.push_back( encoding[i] );
        } else {
            //serialize first n-1 bytes
            for(size_t i = 0; i < encoding.bytes() - 1; i++) {
                data.back() |= ( (encoding[i] & leftByteMasks[8 - last_bit]) >> last_bit );
                data.push_back( (encoding[i] & rightByteMasks[last_bit]) << (8 - last_bit) );
            }

            //serialize last byte
            data.back() |= ( (encoding[encoding.bytes() - 1] & leftByteMasks[8 - last_bit]) >> last_bit );

            if (last_bit + encoding.last_byte_bits() > 8) {
                data.push_back( (encoding[encoding.bytes() - 1] & rightByteMasks[last_bit]) << (8 - last_bit) );
            }
        }

        last_bit = (last_bit + encoding.last_byte_bits()) % 8;
        if (last_bit == 0) last_bit = 8;
    }
}

#endif