#ifndef HUFFMAN_DECODER_TREE
#define HUFFMAN_DECODER_TREE

#include <vector>
#include <memory>

#include "../definitions.h"

namespace huffman::decoder
{
    struct decoderNode {
        char character;
        std::unique_ptr<decoderNode> trueChild;
        std::unique_ptr<decoderNode> falseChild;

        decoderNode(char character);
        decoderNode(std::unique_ptr<decoderNode>&& trueChild, std::unique_ptr<decoderNode>&& falseChild);

        bool is_leaf() const;
    };

    class decoderTree {
    private:
        decoderNode root;

    public:
        decoderTree(const std::vector<byte>& encoded_table);

        char decode(std::vector<bool>::const_iterator& bit_stream) const;
    };
}

#endif