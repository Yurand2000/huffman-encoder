#ifndef HUFFMAN_DECODER_TREE
#define HUFFMAN_DECODER_TREE

#include <vector>
#include <memory>

#include "../bit_stream.h"
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
        decoderTree(std::vector<byte>::const_iterator& encoded_table);
        decoderTree(std::vector<byte>::const_iterator&& encoded_table);

        char decode(bitStream& bit_stream) const;
    };
}

#endif