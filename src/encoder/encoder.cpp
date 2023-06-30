#include "encoder.h"

#include "encoder_table.h"
#include "../utils.h"

namespace huffman::encoder::detail
{

}

namespace huffman::encoder
{
    std::vector<byte> encode(std::string text) {
        auto table = encoderTable(text);

        auto out_data = table.serialize();

        auto bits = std::vector<bool>();
        for (char character : text) {
            auto encoding = table.get(static_cast<byte>(character));

            for(size_t i = encoding.bits; i > 0; i--) {
                bits.push_back(encoding.code[i-1]);
            }
        }

        auto number_of_bytes = positive_div_ceil(bits.size(), static_cast<size_t>(8));
        out_data.reserve(out_data.size() + sizeof(size_t) + number_of_bytes);

        //insert the number of characters
        auto number_of_characters = text.size();
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            out_data.push_back(size_bytes[i]);
        }

        //encode text
        for (size_t i = 0; i < number_of_bytes; i++ ) {
            byte current_byte = 0;
            for(size_t j = 0; j < 8 && (i*8+j) < bits.size(); j++) {
                current_byte |= bits[i*8 + j] << (7 - j);
            }

            out_data.push_back(current_byte);
        }

        return out_data;
    }
}