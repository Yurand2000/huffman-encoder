#include "decoder_tree.h"

#include "../test_utils.h"

#include "../utils.h"
#include "../encoder/encoder_table.h"

using namespace huffman::decoder;

void testBuildDecoderTree() {
    using namespace huffman::encoder;

    auto input = "this is an example of a huffman tree";
    auto table = encoderTable(input);
    auto serialized = table.serialize();
    auto decoder = decoderTree(serialized.cbegin());

    for(size_t i = 0; i < TABLE_SIZE; i++) {
        auto& current = table.get(i);
        if (current.bits == 0) continue;

        auto number_of_bytes = positive_div_ceil(current.bits, static_cast<byte>(8));
        auto text = std::vector<byte>(number_of_bytes);
        for(size_t j = current.bits; j > 0; j--) {
            auto current_byte = (current.bits - j) / 8;
            auto current_bit = 7 - ((current.bits - j) % 8);
            text[current_byte] |= current.code[j-1] << current_bit;
        }

        auto bit_stream = huffman::bitStream(text);
        auto decoded = decoder.decode(bit_stream);
        assert(decoded == static_cast<char>(i), "Expected to find character \'", std::string(1, static_cast<char>(i)) ,"\' but decoded \'", std::string(1, decoded), "\'.");
    }
}

void testMain()
{
    testBuildDecoderTree();
}