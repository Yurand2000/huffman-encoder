#ifndef HUFFMAN_ENCODER
#define HUFFMAN_ENCODER

#include <vector>
#include <string>

namespace huffman::encoder
{
    std::vector<bool> encode(std::string text);
}

#endif