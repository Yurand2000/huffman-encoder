#include "encoder.h"

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
        characterSerializer(const encoderTable& table, std::vector<byte>& data);

        inline void append(char character);
    };

    characterSerializer::characterSerializer(const encoderTable& table, std::vector<byte>& data)
        : table(table), data(data), last_bit(8) {}

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
        auto serializer = detail::characterSerializer(table, out_data);
        for (char character : text)
            serializer.append(character);

        return out_data;
    }

    std::vector<byte> encode_parallel_native(std::string text, size_t workers) {
        return std::vector<byte>();
    }

    std::vector<byte> encode_parallel_ff(std::string text, size_t workers) {
        return std::vector<byte>();
    }
}