#ifndef HUFFMAN_ENCODER
#define HUFFMAN_ENCODER

#include <vector>
#include <string>

#include "../definitions.h"

namespace huffman::encoder
{
    std::vector<byte> encode(std::string text);

    std::vector<byte> encode_parallel_native(std::string text, size_t workers);

    std::vector<byte> encode_parallel_ff(std::string text, size_t workers);
}

#endif