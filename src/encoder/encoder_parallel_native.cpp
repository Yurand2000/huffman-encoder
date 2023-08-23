#include "encoder.h"

#include "encoder_table.h"
#include "character_serializer.h"
#include "../utils.h"

#include "../threads/threadTask.h"

#ifdef CHRONO_ENABLED
#include "../timing.h"
#endif

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

    size_t compute_segment_size(
        std::string const& text,
        size_t workers
    ) {
        return positive_div_ceil(text.length(), workers);
    }

    std::pair<std::string::const_iterator, std::string::const_iterator> extract_task_range(
        std::string const& text,
        size_t segment_size,
        size_t workers,
        size_t worker_num
    ) {
        auto begin = text.cbegin() + segment_size * worker_num;
        auto end = (worker_num == workers - 1) ? text.cend() : begin + segment_size;

        return { begin, end };
    }

    void combine_frequencies(
        std::unordered_map<char, int>& total_frequencies,
        std::unordered_map<char, int> const& partial_frequencies
    ) {
        for(auto const& pair : partial_frequencies) {
            if (total_frequencies.find(pair.first) != total_frequencies.end())
                total_frequencies[pair.first] += pair.second;
            else
                total_frequencies.emplace(pair.first, pair.second);
        }
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
        auto segment_size = compute_segment_size(text, workers);
        auto fun = std::function(&extract_frequencies);
        for(size_t i = 0; i < workers; i++) {
            auto pair = extract_task_range(text, segment_size, workers, i);
            work_threads[i] = submitTask(std::move(threads[i]), fun, pair.first, pair.second);
        }

        //compute total frequencies (reduce)
        frequencies.resize(workers);
        for(size_t i = 0; i < workers; i++) {
            threads[i] = getResult(std::move(work_threads[i]), frequencies[i]);
            combine_frequencies(total_frequencies, frequencies[i]);
        }
    }

    void append_text_parallel(
        std::vector<byte>& out_data,
        std::vector<byte>& data_to_append,
        byte offset
    ) {
        if (data_to_append.empty()) return;

        if (offset == 0) {
            out_data.insert(out_data.end(), data_to_append.begin(), data_to_append.end());
        } else {
            out_data.back() |= data_to_append[0];
            out_data.insert(out_data.end(), data_to_append.begin() + 1, data_to_append.end());
        }
    }

    void compute_serialization_offsets(
        encoderTable const& table,
        std::vector<std::unordered_map<char, int>> const& frequencies,
        std::vector<byte>& offsets,
        size_t workers
    ) {
        offsets.resize(workers);
        offsets[0] = 0;
        for(size_t i = 1; i < workers; i++) {
            auto previous_bits = count_bits(table, frequencies[i - 1]);
            auto previous_offset = offsets[i - 1];
            offsets[i] = ((previous_bits + previous_offset) % 8);
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
        std::vector<byte> offsets;
        compute_serialization_offsets(table, frequencies, offsets, workers);

        //submit tasks (map)
        auto segment_size = compute_segment_size(text, workers);
        for(size_t i = 0; i < workers; i++) {
            auto pair = extract_task_range(text, segment_size, workers, i);
            work_threads[i] = submitTask(std::move(threads[i]), fun, pair.first, pair.second, offsets[i]);
        }

        //append serialized text (reduce)
        for(size_t i = 0; i < workers; i++) {
            std::vector<byte> data;
            threads[i] = getResult(std::move(work_threads[i]), data);
            detail::append_text_parallel(out_data, data, offsets[i]);
        }
    }
}

namespace huffman::encoder
{
    std::vector<byte> encode_parallel_native(std::string text, size_t workers) {
#ifdef CHRONO_ENABLED
        auto& timing = TimingLogger::instance();
        auto& thread_spawn_timer = timing.newTimer("02.** - Thread spawning.");
#endif

        //spawn necessary threads
        auto threads = detail::spawnThreads(workers);

#ifdef CHRONO_ENABLED
        thread_spawn_timer.stopTimer();
        auto& frequencies_timer = timing.newTimer("02.00 - Extracting letter frequencies from the text (parallel).");
#endif

        //extract frequencies of letters (parallelized)
        std::unordered_map<char, int> total_frequencies;
        std::vector<std::unordered_map<char, int>> frequencies;
        detail::extract_frequencies_parallel(threads, total_frequencies, frequencies, text, workers);

#ifdef CHRONO_ENABLED
        frequencies_timer.stopTimer();
        auto& encodingTable_timer = timing.newTimer("02.01 - Building encoding table.");
#endif

        //build the encoding table
        auto table = encoderTable(total_frequencies);

#ifdef CHRONO_ENABLED
        encodingTable_timer.stopTimer();
        auto& serialization_timer = timing.newTimer("02.02 - Serialization of text.");
        auto& serialize_metadata_timer = timing.newTimer("02.02a - Serialization of metadata.");
#endif

        //serialize the table in the output array
        auto out_data = table.serialize();

        //insert the number of characters
        detail::append_text_metadata(text, out_data);

#ifdef CHRONO_ENABLED
        serialize_metadata_timer.stopTimer();
        auto& serialize_text_timer = timing.newTimer("02.02b - Serialization of actual text (parallel).");
#endif

        //encode text (parallelized)
        detail::encode_text_parallel(threads, frequencies, out_data, table, text, workers);

#ifdef CHRONO_ENABLED
        serialize_text_timer.stopTimer();
        serialization_timer.stopTimer();
#endif

        return out_data;
    }
}