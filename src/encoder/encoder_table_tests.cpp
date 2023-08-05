#include "encoder_table.h"

#include "../test_utils.h"

#include "serializable_character.h"

using namespace huffman::encoder;

void generateEncoderTable()
{
    //frequencies for the string: this is an example of a huffman tree
    auto frequencies = std::unordered_map<char, int>{
        {' ', 7}, {'a', 4}, {'e', 4}, {'f', 3}, {'h', 2}, {'i', 2}, {'m', 2}, {'n', 2},
        {'s', 2}, {'t', 2}, {'l', 1}, {'o', 1}, {'p', 1}, {'r', 1}, {'u', 1}, {'x', 1},
    };
    auto& table = encoderTable(frequencies).get_table();

    //verify that the generated codes are prefix-free codes
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        auto& current = table[i];
        if (current.bits == 0) continue;

        for (size_t j = 0; j < TABLE_SIZE; j++) {
            if (i == j) continue;

            auto& other = table[j];
            if (other.bits == 0) continue;
            assert(!current.is_prefix(other),
                "Code of character \'", std::string(1, static_cast<char>(i)), "\' : ", current.to_string(),
                " is a prefix of character \'", std::string(1, static_cast<char>(j)), "\' : ", other.to_string());
        }
    }
}

void testSerialization()
{
    //frequencies for the string: this is an example of a huffman tree
    auto frequencies = std::unordered_map<char, int>{
        {' ', 7}, {'a', 4}, {'e', 4}, {'f', 3}, {'h', 2}, {'i', 2}, {'m', 2}, {'n', 2},
        {'s', 2}, {'t', 2}, {'l', 1}, {'o', 1}, {'p', 1}, {'r', 1}, {'u', 1}, {'x', 1},
    };
    auto table = encoderTable(frequencies);

    auto serialized = table.serialize();

    auto valid_characters = 0;
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        if (table.get(i).bits > 0)
            valid_characters++;
    }

    assert(serialized[0] == valid_characters, "Expected number \'", valid_characters, "\' as first byte,",
        " representing the number of chracters, but found: ", std::to_string(serialized[0]));

    auto iter = serialized.cbegin(); iter++; //the first byte is the number of characters
    for(size_t i = 0; i < valid_characters; i++) {
        auto deserialized = serializableCharacter(iter);
        auto original_encoding = table.get(deserialized.character);
        assert(original_encoding == deserialized.encoding, "Expected deserialized character \'", deserialized.character , "\' (code ",
            std::to_string(deserialized.character), ") in table. "
            "Expected encoding: ", original_encoding.to_string(), "found instead: ", deserialized.encoding.to_string());
    }
}

void testMain()
{
    generateEncoderTable();
    testSerialization();
}