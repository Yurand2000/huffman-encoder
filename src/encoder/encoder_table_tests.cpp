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
    assert(encoded.code.size() == 1, "Expected 1 byte containing the bits");
    assert((encoded.code[0] & 0b10000000) == 0b10000000, "Expected first bit to be true, but found: ", std::bitset<8>(encoded.code[0]));

    encoded = encodedCharacter();
    encoded.append_bit(false);

    assert(encoded.bits == 1, "Expected 1 bit");
    assert(encoded.code.size() == 1, "Expected 1 byte containing the bits");
    assert((encoded.code[0] & 0b00000000) == 0b00000000, "Expected first bit to be false, but found: ", std::bitset<8>(encoded.code[0]));
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
    assert(encoded.code.size() == 1, "Expected 1 byte containing the bits");
    assert((encoded.code[0] & 0b11111000) == 0b10110000, "Expected sequence 10110, but found: ", std::bitset<8>(encoded.code[0]));
}

void appendBitsMoreThanEight()
{
    using namespace huffman::encoder;
    
    auto encoded = encodedCharacter();
    encoded.bits = 8;
    encoded.code.push_back(0b10010011);
    encoded.append_bit(true);
    encoded.append_bit(true);
    encoded.append_bit(false);

    assert(encoded.bits == 11, "Expected 1 bits");
    assert(encoded.code.size() == 2, "Expected 2 bytes containing the bits");
    assert((encoded.code[0] & 0b11111111) == 0b10010011, "Expected first byte to be 10010011, but found: ", std::bitset<8>(encoded.code[0]));
    assert((encoded.code[1] & 0b11111111) == 0b11000000, "Expected second byte to be 110, but found: ", std::bitset<8>(encoded.code[1]));
}

int main()
{
    appendBitOnEmpty();
    appendBits();
    appendBitsMoreThanEight();

    return 0;
}