#include "decoder.h"

#include "decoder_tree.h"
#include "../bit_stream.h"
#include "../utils.h"

namespace huffman::decoder
{
    std::string decode(const std::vector<byte>& encoded_text)
    {
        auto iter = encoded_text.cbegin();

        //decode encodings
        auto decoder = decoderTree(iter);

        //get the number of characters
        size_t number_of_characters = 0;
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            size_bytes[i] = *iter; iter++;
        }

        //decode characters
        auto offset = std::distance(encoded_text.cbegin(), iter);

        auto bit_stream = bitStream(encoded_text);
        bit_stream.advance(offset * 8);

        auto string = std::string();
        for(size_t i = 0; i < number_of_characters && bit_stream.hasNext(); i++) {
            auto character = decoder.decode(bit_stream);
            string += character;
        }

        return string;
    }
}