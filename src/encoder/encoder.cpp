#include "encoder.h"

#include "encoder_table.h"
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
    std::vector<byte> encode(std::string text) {
        auto table = encoderTable(text);

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        auto number_of_characters = text.size();
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            out_data.push_back(size_bytes[i]);
        }

        //encode text
        size_t last_bit = 0;
        for (char character : text) {
            auto& encoding = table.get(static_cast<byte>(character));

            for(size_t i = 0; i < encoding.bytes() - 1; i++) {
                if (last_bit == 0) {
                    out_data.push_back( encoding.code[i] );
                } else {
                    out_data.back() |= ( (encoding.code[i] & detail::leftByteMasks[8 - last_bit]) >> last_bit );
                    out_data.push_back( (encoding.code[i] & detail::rightByteMasks[last_bit]) << (8 - last_bit) );
                }
            }

            if (last_bit == 0) {
                out_data.push_back(encoding.code[encoding.bytes() - 1]);
            } else {
                out_data.back() |= ( (encoding.code[encoding.bytes() - 1] & detail::leftByteMasks[8 - last_bit]) >> last_bit );

                if (last_bit + encoding.last_byte_bits() > 8) {
                    out_data.push_back( (encoding.code[encoding.bytes() - 1] & detail::rightByteMasks[last_bit]) << (8 - last_bit) );
                }
            }

            last_bit = (last_bit + encoding.last_byte_bits()) % 8;
        }

        return out_data;
    }
}