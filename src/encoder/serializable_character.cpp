#include "serializable_character.h"

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <bitset>

#include "../utils.h"

namespace huffman::encoder::detail
{    
    inline encodedCharacter deserialize_character(std::vector<byte>::const_iterator& bytes, char& character)
    {
        character = static_cast<char>(*bytes);

        auto encoding = encodedCharacter();
        encoding.bits = *(++bytes);

        auto number_of_bytes = positive_div_ceil<byte>(encoding.bits, 8);

        auto curr_byte = *(++bytes);
        for (size_t i = number_of_bytes; i > 0; i--) {
            for (size_t j = 0; j < 8; j++)
                encoding.code[ (i-1)*8 + j ] = (curr_byte & (1 << j)) >> j;
            curr_byte = *(++bytes);
        }
        
        return encoding;
    }
}

namespace huffman::encoder
{
    serializableCharacter::serializableCharacter(char character, encodedCharacter encoding)
        : character(character), encoding(encoding) { }
    
    serializableCharacter::serializableCharacter(std::vector<byte>::const_iterator& bytes) {
        encoding = detail::deserialize_character(bytes, character);
    }

    std::vector<byte> serializableCharacter::serialize() const {
        auto bytes = positive_div_ceil<byte>(encoding.bits, 8);

        auto serialized = std::vector<byte>();
        serialized.reserve(2 + bytes);
        serialized.push_back(static_cast<byte>(character));
        serialized.push_back(encoding.bits);

        for (size_t i = bytes; i > 0; i--) {
            byte code = 0;
            for (size_t j = 0; j < 8; j++)
                code |= encoding.code[ (i-1)*8 + j ] << j;
            serialized.push_back(code);
        }

        return serialized;
    }
}