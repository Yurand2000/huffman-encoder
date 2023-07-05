#include "encoded_character.h"

#include "../test_utils.h"

#include <iostream>
#include <bitset>

using namespace huffman::encoder;

void getBits()
{
    auto encoded = encodedCharacter();
    encoded.bits = 5;
    encoded.code[0] = 0b11010000;

    assert(encoded.get_bit(1) == true, "Expected first bit to be true, but found: ", encoded.to_string());
    assert(encoded.get_bit(3) == false, "Expected third bit to be true, but found: ", encoded.to_string());
    assert(encoded.get_bit(4) == true, "Expected 4th bit to be true, but found: ", encoded.to_string());
}

void appendBitOnEmpty()
{
    auto encoded = encodedCharacter();
    encoded.append_bit(true);

    assert(encoded.bits == 1, "Expected 1 bit");
    assert(encoded.code[0] == 0b10000000, "Expected first bit to be true, but found: ", encoded.to_string());

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
    assert(encoded.code[0] == 0b10110000, "Expected sequence 10110, but found: ", encoded.to_string());
}

void appendBitsMoreThanEight()
{    
    auto encoded = encodedCharacter();
    encoded.bits = 8;
    encoded.code[0] = 0b10010011;
    encoded.append_bit(true);
    encoded.append_bit(true);
    encoded.append_bit(false);

    assert(encoded.bits == 11, "Expected 11 bits");
    assert(encoded.code[0] == 0b10010011 && encoded.code[1] == 0b11000000, "Expected sequence 10010011 110, but found: ", encoded.to_string());
}

void testSerialization()
{
    auto encoded = encodedCharacter();
    encoded.bits = 10;
    encoded.code[0] = 0b11100100;
    encoded.code[1] = 0b11000000;

    auto serialized = encoded.serialize();
    auto deserialized = encodedCharacter(serialized.cbegin());
    assert(encoded == deserialized, "Expected deserialized sequence 11100100 11, but found: ", deserialized.to_string());
}

void testIsPrefix()
{
    auto char0 = encodedCharacter();
    char0.bits = 3; char0.code[0] = 0b10100000;

    auto char1 = encodedCharacter();
    char1.bits = 4; char1.code[0] = 0b10110000;

    assert(char0.is_prefix(char1), "Expected first character, of code ", char0.to_string(), " to be a prefix of character ", char1.to_string());
}

void testMain()
{
    getBits();
    appendBitOnEmpty();
    appendBits();
    appendBitsMoreThanEight();
    testSerialization();
    testIsPrefix();
}