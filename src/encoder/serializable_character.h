#ifndef HUFFMAN_SERIALIZABLE_CHARACTER
#define HUFFMAN_SERIALIZABLE_CHARACTER

#include "../definitions.h"

#include "encoded_character.h"

namespace huffman::encoder
{
    struct serializableCharacter {
        char character;
        encodedCharacter encoding;

        serializableCharacter(char character, encodedCharacter encoding);
        serializableCharacter(std::vector<byte>::const_iterator& bytes);
        serializableCharacter(std::vector<byte>::const_iterator&& bytes);

        std::vector<byte> serialize() const;
    };
}

#endif