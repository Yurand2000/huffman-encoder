#include "encoder.h"

#include "encoder_table.h"
#include "character_serializer.h"
#include "../utils.h"

#include "../threads/threadTask.h"

namespace huffman::encoder::detail
{
    using namespace huffman::encoder;
    using namespace huffman::parallel::native;

    std::unordered_map<char, int> extract_frequencies(std::string::const_iterator, std::string::const_iterator);

    size_t count_bits(const encoderTable&, std::unordered_map<char, int>);

    void append_text_metadata(std::string const&, std::vector<byte>&);

    std::vector<byte> encode_text(const encoderTable&, std::string::const_iterator, std::string::const_iterator, byte);

    std::vector<threadTask> spawnThreads(size_t workers) {
        std::vector<threadTask> threads(workers);
        for(size_t i = 0; i < workers; i++) {
            threads[i] = spawnThread();
        }

        return threads;
    }

    void extract_frequencies_parallel(
        std::vector<threadTask>& threads,
        std::unordered_map<char, int>& total_frequencies,
        std::vector<std::unordered_map<char, int>>& frequencies,
        std::string const& text,
        size_t workers
    ) {
        using threadResultFrequencies =
            threadResult<
                std::unordered_map<char, int>,
                std::string::const_iterator,
                std::string::const_iterator
            >;

        std::vector<threadResultFrequencies> work_threads(workers);

        //submit tasks (map)
        auto segment_size = positive_div_ceil(text.length(), workers);
        auto fun = std::function(&extract_frequencies);
        for(size_t i = 0; i < workers; i++) {
            auto begin = text.cbegin() + segment_size * i;
            auto end = (i == workers - 1) ? text.cend() : begin + segment_size;
            work_threads[i] = submitTask(std::move(threads[i]), fun, begin, end);
        }

        //compute total frequencies (reduce)
        frequencies.resize(workers);
        for(size_t i = 0; i < workers; i++) {
            threads[i] = getResult(std::move(work_threads[i]), frequencies[i]);

            for(auto pair : frequencies[i]) {
                if (total_frequencies.find(pair.first) != total_frequencies.end())
                    total_frequencies[pair.first] += pair.second;
                else
                    total_frequencies.emplace(pair.first, pair.second);
            }
        }
    }

    void encode_text_parallel(
        std::vector<threadTask>& threads,
        std::vector<std::unordered_map<char, int>>& frequencies,
        std::vector<byte>& out_data,
        encoderTable const& table,
        std::string const& text,
        size_t workers
    ) {
        using threadResultEncoding = threadResult<
            std::vector<byte>,
            std::string::const_iterator,
            std::string::const_iterator,
            byte
        >;

        std::vector<threadResultEncoding> work_threads(workers);

        //wrapper function which captures the local environment
        auto fun = std::function([table](
            std::string::const_iterator text_start,
            std::string::const_iterator text_end,
            byte offset
        ) {
            return encode_text(table, text_start, text_end, offset);
        });

        //compute serialization offsets
        std::vector<byte> offsets(workers);
        
        offsets[0] = 0;
        for(size_t i = 1; i < workers; i++) {
            auto previous_bits = count_bits(table, frequencies[i - 1]);
            auto previous_offset = offsets[i - 1];
            offsets[i] = ((previous_bits + previous_offset) % 8);
        }

        //submit tasks (map)
        auto segment_size = positive_div_ceil(text.length(), workers);
        for(size_t i = 0; i < workers; i++) {
            auto begin = text.cbegin() + segment_size * i;
            auto end = (i == workers - 1) ? text.cend() : begin + segment_size;
            work_threads[i] = submitTask(std::move(threads[i]), fun, begin, end, offsets[i]);
        }

        //append serialized text (reduce)
        for(size_t i = 0; i < workers; i++) {
            std::vector<byte> data;
            threads[i] = getResult(std::move(work_threads[i]), data);

            if (data.empty()) continue;

            if (offsets[i] == 0) {
                out_data.insert(out_data.end(), data.begin(), data.end());
            } else {
                out_data.back() |= data[0];
                out_data.insert(out_data.end(), data.begin() + 1, data.end());
            }
        }
    }
}

namespace huffman::encoder
{
    std::vector<byte> encode_parallel_native(std::string text, size_t workers) {

        //spawn necessary threads
        auto threads = detail::spawnThreads(workers);

        //extract frequencies of letters (parallelized)
        std::unordered_map<char, int> total_frequencies;
        std::vector<std::unordered_map<char, int>> frequencies;
        detail::extract_frequencies_parallel(threads, total_frequencies, frequencies, text, workers);

        //build the encoding table
        auto table = encoderTable(total_frequencies);

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        detail::append_text_metadata(text, out_data);

        //encode text (parallelized)
        detail::encode_text_parallel(threads, frequencies, out_data, table, text, workers);

        return out_data;
    }
}