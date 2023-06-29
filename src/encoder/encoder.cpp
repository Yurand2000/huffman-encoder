#include "encoder.h"

#include "encoder_table.h"

namespace huffman::encoder
{
    std::vector<bool> encode(std::string text) {
        auto table = encoderTable(text);

        auto bits = std::vector<bool>();
        for (char character : text) {
            auto encoding = table.get(static_cast<byte>(character));

            for(size_t i = encoding.bits; i > 0; i--) {
                bits.push_back(encoding.code[i-1]);
            }
        }

        return bits;
    }
}