#ifndef HUFFMAN_ENCODER
#define HUFFMAN_ENCODER

#include <vector>
#include <string>

#include "../definitions.h"

namespace huffman::encoder
{
    std::vector<byte> encode(std::string text);
}

#endif