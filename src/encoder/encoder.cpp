#include "encoder.h"

#include "encoder_table.h"
#include "../utils.h"

#include "../threads/threadTask.h"

namespace huffman::encoder::detail
{
    using namespace huffman::encoder;

    struct characterSerializer {
    private:
        const encoderTable& table;
        std::vector<byte>& data;
        byte last_bit;

    public:
        characterSerializer(const encoderTable& table, std::vector<byte>& data, byte offset = 8);

        inline void append(char character);
    };

    characterSerializer::characterSerializer(const encoderTable& table, std::vector<byte>& data, byte offset)
        : table(table), data(data)
    {
        if (offset == 0 || offset > 8) offset = 8;
        if (offset != 8) data.push_back(0);
        last_bit = offset;
    }

    void characterSerializer::append(char character) {
        auto& encoding = table.get(character);

        if (last_bit == 8) {
            //just append the character encoding, because it is right aligned.
            for(size_t i = 0; i < encoding.bytes(); i++)
                data.push_back( encoding[i] );
        } else {
            //serialize first n-1 bytes
            for(size_t i = 0; i < encoding.bytes() - 1; i++) {
                data.back() |= ( (encoding[i] & leftByteMasks[8 - last_bit]) >> last_bit );
                data.push_back( (encoding[i] & rightByteMasks[last_bit]) << (8 - last_bit) );
            }

            //serialize last byte
            data.back() |= ( (encoding[encoding.bytes() - 1] & leftByteMasks[8 - last_bit]) >> last_bit );

            if (last_bit + encoding.last_byte_bits() > 8) {
                data.push_back( (encoding[encoding.bytes() - 1] & rightByteMasks[last_bit]) << (8 - last_bit) );
            }
        }

        last_bit = (last_bit + encoding.last_byte_bits()) % 8;
        if (last_bit == 0) last_bit = 8;
    }

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
}

namespace huffman::encoder
{
    std::vector<byte> encode(std::string text) {
        auto frequencies = detail::extract_frequencies(text.cbegin(), text.cend());
        auto table = encoderTable(frequencies);

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        auto number_of_characters = text.size();
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            out_data.push_back(size_bytes[i]);
        }

        //encode text
        auto serializer = detail::characterSerializer(table, out_data);
        for (char character : text)
            serializer.append(character);

        return out_data;
    }

    std::vector<byte> encode_parallel_native(std::string text, size_t workers) {
        using namespace huffman::parallel::native;

        std::vector<threadTask> threads(workers);
        for(size_t i = 0; i < workers; i++) {
            threads[i] = spawnThread();
        }

        std::vector<threadResult<
            std::unordered_map<char, int>,
            std::string::const_iterator,
            std::string::const_iterator
        >> work_threads(workers);

        auto segment_size = positive_div_ceil(text.length(), workers);
        auto fun = std::function(&detail::extract_frequencies);
        for(size_t i = 0; i < workers; i++) {
            auto begin = text.cbegin() + segment_size * i;
            auto end = (i == workers - 1) ? text.cend() : begin + segment_size;
            work_threads[i] = submitTask(std::move(threads[i]), fun, begin, end);
        }

        std::unordered_map<char, int> total_frequencies;
        std::vector<std::unordered_map<char, int>> frequencies(workers);
        for(size_t i = 0; i < workers; i++) {
            threads[i] = getResult(std::move(work_threads[i]), frequencies[i]);

            for(auto pair : frequencies[i]) {
                if (total_frequencies.find(pair.first) != total_frequencies.end())
                    total_frequencies[pair.first] += pair.second;
                else
                    total_frequencies.emplace(pair.first, pair.second);
            }
        }

        auto table = encoderTable(total_frequencies);

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        auto number_of_characters = text.size();
        auto size_bytes = reinterpret_cast<byte*>(&number_of_characters);
        for(size_t i = 0; i < sizeof(size_t); i++) {
            out_data.push_back(size_bytes[i]);
        }

        //encode text
        std::vector<threadResult<
            std::vector<byte>,
            std::string::const_iterator,
            std::string::const_iterator,
            byte
        >> work_threads2(workers);

        auto fun2 = std::function([table](
            std::string::const_iterator text_start,
            std::string::const_iterator text_end,
            byte offset
        ) {
            std::vector<byte> out_data;
            auto serializer = detail::characterSerializer(table, out_data, offset);

            for (auto iter = text_start; iter != text_end; iter++) {
                serializer.append(*iter);
            }

            return out_data;
        });

        std::vector<byte> offsets(workers);
        for(size_t i = 0; i < workers; i++) {
            auto begin = text.cbegin() + segment_size * i;
            auto end = (i == workers - 1) ? text.cend() : begin + segment_size;

            if (i == 0) {
                offsets[i] = 0;
            } else {
                auto previous_bits = detail::count_bits(table, frequencies[i - 1]);
                auto previous_offset = offsets[i - 1];
                offsets[i] = ((previous_bits + previous_offset) % 8);
            }
            work_threads2[i] = submitTask(std::move(threads[i]), fun2, begin, end, offsets[i]);
        }

        for(size_t i = 0; i < workers; i++) {
            std::vector<byte> data;
            threads[i] = (getResult(std::move(work_threads2[i]), data));

            if (data.empty()) continue;

            if (offsets[i] == 0) {
                out_data.insert(out_data.end(), data.begin(), data.end());
            } else {
                out_data.back() |= data[0];
                out_data.insert(out_data.end(), data.begin() + 1, data.end());
            }
        }

        return out_data;
    }

    std::vector<byte> encode_parallel_ff(std::string text, size_t workers) {
        return std::vector<byte>();
    }
}