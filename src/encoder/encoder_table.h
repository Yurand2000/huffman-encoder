#ifndef HUFFMAN_ENCODER_TABLE
#define HUFFMAN_ENCODER_TABLE

#include <string>
#include <vector>

#include "../definitions.h"
#include "encoded_character.h"

namespace huffman::encoder
{
    class encoderTable {
        private:
            encodedCharacter table[TABLE_SIZE];

            encoderTable();

        public:
            encoderTable(const std::string& text);

            inline const encodedCharacter& get(char character) const {
                return table[static_cast<byte>(character)];
            }

            inline encodedCharacter& get_mut(char character) {
                return table[static_cast<byte>(character)];
            }

            inline const encodedCharacter (&get_table() const)[TABLE_SIZE] {
                return table;
            }

            std::vector<byte> serialize() const;
            std::string to_string() const;
    };
}

#endif