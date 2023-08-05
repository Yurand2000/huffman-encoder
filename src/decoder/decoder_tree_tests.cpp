#include "decoder_tree.h"

#include "../test_utils.h"

#include "../utils.h"
#include "../encoder/encoder_table.h"

using namespace huffman::decoder;

void testBuildDecoderTree() {
    using namespace huffman::encoder;

    //frequencies for the string: this is an example of a huffman tree
    auto frequencies = std::unordered_map<char, int>{
        {' ', 7}, {'a', 4}, {'e', 4}, {'f', 3}, {'h', 2}, {'i', 2}, {'m', 2}, {'n', 2},
        {'s', 2}, {'t', 2}, {'l', 1}, {'o', 1}, {'p', 1}, {'r', 1}, {'u', 1}, {'x', 1},
    };
    auto table = encoderTable(frequencies);
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