#include "serializable_character.h"

#include "../test_utils.h"

#include <iostream>
#include <bitset>

using namespace huffman::encoder;

void testSerialization0()
{
    auto encoding = encodedCharacter();
    encoding.bits = 5;
    encoding.code[2] = true; encoding.code[4] = true; //0b10100

    auto original = serializableCharacter('f', encoding);
    auto serialization = original.serialize();
    auto deserialized = serializableCharacter(serialization.cbegin());
    assert(original.character == deserialized.character, "Expected deserialized character to be \'f\', but found: ", deserialized.character);
    assert(original.encoding == deserialized.encoding, "Expected deserialized character code to be 10100, but found: ", deserialized.encoding.to_string());
}

void testMain()
{
    testSerialization0();
}