#include "encoder_table.h"
#include "../utils.h"

#include <bitset>

template<typename... Args>
inline void assert(bool expression, Args&&... args) {
    assert_fn(expression, "encoder_table_tests.cpp", std::forward<Args>(args)...);
}

//encoded character tests
void appendBitOnEmpty()
{
    using namespace huffman::encoder;

    auto encoded = encodedCharacter();
    encoded.append_bit(true);

    assert(encoded.bits == 1, "Expected 1 bit");
    assert(encoded.code[0] == 1, "Expected first bit to be true, but found: ", encoded.to_string());

    encoded = encodedCharacter();
    encoded.append_bit(false);

    assert(encoded.bits == 1, "Expected 1 bit");
    assert(encoded.code[0] == 0, "Expected first bit to be false, but found: ", encoded.to_string());
}

void appendBits()
{
    using namespace huffman::encoder;

    auto encoded = encodedCharacter();
    encoded.append_bit(true);
    encoded.append_bit(false);
    encoded.append_bit(true);
    encoded.append_bit(true);
    encoded.append_bit(false);
    
    assert(encoded.bits == 5, "Expected 5 bits");
    assert(encoded.code == std::bitset<TABLE_SIZE>(0b00010110), "Expected sequence 10110, but found: ", encoded.to_string());
}

void appendBitsMoreThanEight()
{
    using namespace huffman::encoder;
    
    auto encoded = encodedCharacter();
    encoded.bits = 8;
    encoded.code = std::bitset<TABLE_SIZE>(0b10010011);
    encoded.append_bit(true);
    encoded.append_bit(true);
    encoded.append_bit(false);

    assert(encoded.bits == 11, "Expected 11 bits");
    assert(encoded.code == std::bitset<TABLE_SIZE>(0b10010011110), "Expected sequence 10010011110, but found: ", encoded.to_string());
}

//encoder table generation
void generateEncoderTable()
{
    using namespace huffman::encoder;

    auto input = "this is an example of a huffman tree";
    auto& table = encoderTable(input).get_table();

    //verify that the generated codes are prefix-free codes
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        auto& current = table[i];
        if (current.bits == 0) continue;

        for (size_t j = 0; j < TABLE_SIZE; j++) {
            if (i == j) continue;

            auto& other = table[j];
            if (current.bits <= other.bits) {
                auto suffix_bits = other.bits - current.bits;
                auto other_prefix = other.code >> suffix_bits;
                assert(current.code != other_prefix,
                    "Code of character \'", std::string(1, static_cast<char>(i)), "\' : ", current.to_string(),
                    " is a prefix of character \'", std::string(1, static_cast<char>(j)), "\' : ", other.to_string());
            }
        }
    }
}

//encoder table serialization
void serializeEncoderTable()
{
    using namespace huffman::encoder;

    auto input = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbeeejkadhffhne4oinwmcpowjtuerhhhhhhhhhhhhhhffffffffuuuuuuuddddddddiiiiijjjjjjjjj1569778832/**---+816747328478&!^&!@&&@&($#@!&^&$#&$^@)(&%%@#6735AAAAAAAAAAAAAWWWWWWWWWWWWWWEDSAAAAAACCCCCCMMMMMMMMMMFFFFFIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII7DFSJV MNZ VJKWEBFNKLDMCF SD    JSNSHDFLJSDFI+-*-*/-*/+-/*+/-";
    auto table = encoderTable(input);
    auto serialized = table.serialize();
    auto deserialized = encoderTable(serialized);

    auto original_table = table.get_table();
    auto deserialized_table = deserialized.get_table();

    int max_bits = 0;
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (original_table[i].bits > max_bits)
            max_bits = original_table[i].bits;

        assert(original_table[i].bits == deserialized_table[i].bits && original_table[i].code == deserialized_table[i].code,
            "Serialization of character \'", std::string(1, static_cast<char>(i)), "\' : ", original_table[i].to_string(),
            " does not match its deserialization: ", deserialized_table[i].to_string());
    }

    std::cout << max_bits << std::endl;
}

int main()
{
    appendBitOnEmpty();
    appendBits();
    appendBitsMoreThanEight();

    generateEncoderTable();
    serializeEncoderTable();

    return 0;
}