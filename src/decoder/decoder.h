#ifndef HUFFMAN_DECODER
#define HUFFMAN_DECODER

#include <string>
#include <vector>

#include "../definitions.h"

namespace huffman::decoder
{
    std::string decode(const std::vector<byte>& encoded_text);
}

#endif