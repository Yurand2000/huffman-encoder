#include "decoder_tree.h"

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <bitset>

#include "../encoder/serializable_character.h"
#include "../utils.h"

namespace huffman::decoder::detail
{
    using namespace huffman::encoder;

    std::unique_ptr<decoderNode> buildDecoderTree(std::vector<serializableCharacter> characters, int depth) {
        if (characters.size() == 0)
            throw new std::runtime_error("Given character codes are not prefix free codes");
        else if (characters.size() == 1)
            return std::unique_ptr<decoderNode>( new decoderNode(characters[0].character) );
        else {
            auto trueCharacters = std::vector<serializableCharacter>();
            auto falseCharacters = std::vector<serializableCharacter>();
            for (auto character : characters) {
                if (character.encoding.bits < depth) {
                    throw new std::runtime_error("Given character codes are not prefix free codes");
                } else if (character.encoding.code[character.encoding.bits - depth - 1] == true) {
                    trueCharacters.push_back(character);
                } else {
                    falseCharacters.push_back(character);
                }
            }

            auto trueNode = buildDecoderTree(trueCharacters, depth + 1);
            auto falseNode = buildDecoderTree(falseCharacters, depth + 1);
            return std::unique_ptr<decoderNode>( new decoderNode(std::move(trueNode), std::move(falseNode)) );
        }
    }
    
    char decode(const decoderNode& current_node, bitStream& bit_stream) {
        if (current_node.is_leaf())
            return current_node.character;
        else if (!bit_stream.hasNext())
            return '\0';
        else {
            auto next_bit = *bit_stream; ++bit_stream;
            if (next_bit)
                return decode(*current_node.trueChild, bit_stream);
            else
                return decode(*current_node.falseChild, bit_stream);
        }
    }
}

namespace huffman::decoder
{
    using namespace huffman::encoder;

    decoderNode::decoderNode(char character)
        : character(character), trueChild(nullptr), falseChild(nullptr) {}
    decoderNode::decoderNode(std::unique_ptr<decoderNode>&& trueChild, std::unique_ptr<decoderNode>&& falseChild)
        : character('\0'), trueChild(std::move(trueChild)), falseChild(std::move(falseChild)) { }

    bool decoderNode::is_leaf() const {
        return trueChild == nullptr && falseChild == nullptr;
    }

    decoderTree::decoderTree(std::vector<byte>::const_iterator& encoded_table) : root('\0') {
        auto number_of_characters = *encoded_table; encoded_table++;

        auto characters = std::vector<serializableCharacter>();
        characters.reserve(number_of_characters);

        for (size_t i = 0; i < number_of_characters; i++) {
            characters.push_back( serializableCharacter(encoded_table) );
        }

        root = std::move( *detail::buildDecoderTree(characters, 0).release() );
    }

    decoderTree::decoderTree(std::vector<byte>::const_iterator&& encoded_table)
        : decoderTree(encoded_table) { }
        
    char decoderTree::decode(bitStream& bit_stream) const {
        return detail::decode(root, bit_stream);
    }
}