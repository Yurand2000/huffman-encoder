#include "character_serializer.h"

namespace huffman::encoder::detail
{
    using namespace huffman::encoder;

    characterSerializer::characterSerializer(const encoderTable& table, std::vector<byte>& data, byte offset)
        : table(table), data(data)
    {
        if (offset == 0 || offset > 8) offset = 8;
        if (offset != 8) data.push_back(0);
        last_bit = offset;
    }
}