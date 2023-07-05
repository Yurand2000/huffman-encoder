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

    auto text = std::vector<byte>(TABLE_SIZE_BYTES);
    for(size_t i = 0; i < TABLE_SIZE; i++) {
        auto& current = table.get(i);
        if (current.bits == 0) continue;

        for(size_t j = 0; j < TABLE_SIZE_BYTES; j++) {
            text[j] = current.code[j];
        }

        auto bit_stream = huffman::bitStream(text);
        auto decoded = decoder.decode(bit_stream);
        assert(decoded == static_cast<char>(i),
            "Expected to find character \'", std::string(1, static_cast<char>(i)) ,"\' but decoded \'", std::string(1, decoded), "\'");
    }
}

void testMain()
{
    testBuildDecoderTree();
}