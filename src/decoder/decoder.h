#ifndef HUFFMAN_DECODER
#define HUFFMAN_DECODER

#include <string>
#include <vector>

#include "decoder_tree.h"

namespace huffman::decoder
{
    class decoder {
    private:
        decoderTree _decoder;

    public:
        decoder(const std::vector<byte>& encoded_table);

        std::string decode(std::vector<bool> bit_stream) const;
    };
}

#endif