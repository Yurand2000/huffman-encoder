#include "decoder.h"

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <bitset>

#include "../utils.h"

namespace huffman::decoder
{
    decoder::decoder(const std::vector<byte>& encoded_table)
        : _decoder(encoded_table) {}

    std::string decoder::decode(std::vector<bool> bit_stream) const {
        auto iter = bit_stream.cbegin();
        
        auto string = std::string();
        while(iter != bit_stream.cend()) {
            string += _decoder.decode(iter);
        }

        return string;
    }
}