#ifndef HUFFMAN_ENCODER_TABLE
#define HUFFMAN_ENCODER_TABLE

#include <string>
#include <vector>

#define TABLE_SIZE 256

typedef unsigned char byte;

namespace huffman::encoder
{
    struct encodedCharacter {
        byte bits;
        std::vector<byte> code;

        encodedCharacter();

        void append_bit(bool bit);
    };

    class encoderTable {
        private:
            encodedCharacter table[TABLE_SIZE];

            encoderTable();

        public:
            encoderTable(const std::string& text);
            encoderTable(const std::vector<byte>& encoded_table);

            inline const encodedCharacter& get(char character) const;
            inline encodedCharacter& get_mut(char character);

            std::vector<byte> serialize() const;
    };
}

#endif