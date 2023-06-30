#include "serializable_character.h"

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <bitset>

#include "../utils.h"


namespace huffman::encoder
{
    serializableCharacter::serializableCharacter(char character, encodedCharacter encoding)
        : character(character), encoding(encoding) { }
    
    serializableCharacter::serializableCharacter(std::vector<byte>::const_iterator& bytes) {
        character = static_cast<char>(*bytes);
        encoding = encodedCharacter(++bytes);
    }
    
    serializableCharacter::serializableCharacter(std::vector<byte>::const_iterator&& bytes)
        : serializableCharacter(bytes) {}

    std::vector<byte> serializableCharacter::serialize() const {
        auto serialized = encoding.serialize();
        serialized.insert(serialized.begin(), static_cast<byte>(character));
        return serialized;
    }
}