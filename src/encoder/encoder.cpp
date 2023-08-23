#include "encoder.h"

#include "encoder_table.h"
#include "character_serializer.h"
#include "../utils.h"

#ifdef CHRONO_ENABLED
#include "../timing.h"
#endif

namespace huffman::encoder::detail
{
    using namespace huffman::encoder;

    std::unordered_map<char, int> extract_frequencies(std::string::const_iterator text_start, std::string::const_iterator text_end) {
        auto frequencies = std::unordered_map<char, int>();

        for (auto iter = text_start; iter != text_end; iter++) {
            auto character = *iter;
            if (frequencies.find(character) != frequencies.end()) {
                frequencies[character] += 1;
            } else {
                frequencies.emplace(character, 1);
            }
        }

        return frequencies;
    }

    size_t count_bits(const encoderTable& table, std::unordered_map<char, int> frequencies) {
        size_t bits = 0;
        for(auto pair : frequencies)
            bits += table.get(pair.first).bits * pair.second;

        return bits;
    }

    void append_text_metadata(
        std::string const& text,
        std::vector<byte>& out_data
    ) {
        auto number_of_characters = text.size();
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            out_data.push_back(size_bytes[i]);
        }
    }

    std::vector<byte> encode_text(
        const encoderTable& table,
        std::string::const_iterator text_start,
        std::string::const_iterator text_end,
        byte offset
    ) {
        std::vector<byte> out_data;
        auto serializer = detail::characterSerializer(table, out_data, offset);

        for (auto iter = text_start; iter != text_end; iter++)
            serializer.append(*iter);

        return out_data;
    };
}

namespace huffman::encoder
{
    std::vector<byte> encode(std::string text) {
#ifdef CHRONO_ENABLED
        auto& timing = TimingLogger::instance();
        auto& frequencies_timer = timing.newTimer("02.00 - Extracting letter frequencies from the text.");
#endif
        //extract frequencies of letters
        auto frequencies = detail::extract_frequencies(text.cbegin(), text.cend());

#ifdef CHRONO_ENABLED
        frequencies_timer.stopTimer();
        auto& encodingTable_timer = timing.newTimer("02.01 - Building encoding table.");
#endif
        
        //build the encoding table
        auto table = encoderTable(frequencies);

#ifdef CHRONO_ENABLED
        encodingTable_timer.stopTimer();
        auto& serialization_timer = timing.newTimer("02.02 - Serialization of text.");
#endif

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        detail::append_text_metadata(text, out_data);

        //encode text
        auto serialized = detail::encode_text(table, text.begin(), text.end(), 0);
        out_data.insert(out_data.end(), serialized.begin(), serialized.end());

#ifdef CHRONO_ENABLED
        serialization_timer.stopTimer();
#endif

        return out_data;
    } 
}