#include "encoded_character.h"

#include "../test_utils.h"

#include <iostream>
#include <bitset>

using namespace huffman::encoder;

void appendBitOnEmpty()
{
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
    auto encoded = encodedCharacter();
    encoded.bits = 8;
    encoded.code = std::bitset<TABLE_SIZE>(0b10010011);
    encoded.append_bit(true);
    encoded.append_bit(true);
    encoded.append_bit(false);

    assert(encoded.bits == 11, "Expected 11 bits");
    assert(encoded.code == std::bitset<TABLE_SIZE>(0b10010011110), "Expected sequence 10010011110, but found: ", encoded.to_string());
}

void testSerialization()
{
    auto encoded = encodedCharacter();
    encoded.bits = 10;
    encoded.code = std::bitset<TABLE_SIZE>(0b1110010011);

    auto serialized = encoded.serialize();
    auto deserialized = encodedCharacter(serialized.cbegin());
    assert(encoded.bits == deserialized.bits, "Expected deserialized to have 10 bits, but found ", std::to_string(deserialized.bits), " bits.");
    assert(encoded.code == deserialized.code, "Expected deserialized sequence 1110010011, but found: ", deserialized.to_string());
}

void testMain()
{
    appendBitOnEmpty();
    appendBits();
    appendBitsMoreThanEight();
    testSerialization();
}