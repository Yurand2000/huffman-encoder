#include "encoder.h"

#include "encoder_table.h"
#include "character_serializer.h"
#include "../utils.h"

#include <ff/ff.hpp>

#ifdef CHRONO_ENABLED
#include "../timing.h"
#endif

using namespace ff;

namespace huffman::encoder::detail
{
    std::unordered_map<char, int> extract_frequencies(std::string::const_iterator, std::string::const_iterator);

    size_t count_bits(const encoderTable&, std::unordered_map<char, int>);

    void append_text_metadata(std::string const&, std::vector<byte>&);

    std::vector<byte> encode_text(const encoderTable&, std::string::const_iterator, std::string::const_iterator, byte);

    //parallel function definitions
    void append_text_parallel(std::vector<byte>&, std::vector<byte>&, byte);
    
    size_t compute_segment_size(std::string const&, size_t);

    std::pair<std::string::const_iterator, std::string::const_iterator> extract_task_range(std::string const&, size_t, size_t, size_t);

    void combine_frequencies(std::unordered_map<char, int>&, std::unordered_map<char, int> const&);

    void compute_serialization_offsets(encoderTable const&, std::vector<std::unordered_map<char, int>> const&, std::vector<byte>&, size_t);

    //frequencies extraction farm
    struct frequency_data {
        std::string::const_iterator text_start;
        std::string::const_iterator text_end;
        size_t worker;
    };

    struct frequency_output {
        std::unordered_map<char, int> frequencies;
        size_t worker;
    };

    struct frequencyExtractionEmitter: ff_monode_t<void*, frequency_data>
    {
    private:
        std::string const& text;
        size_t workers;

    public:
        frequencyExtractionEmitter(std::string const& text, size_t workers)
            : text(text), workers(workers) {}

        frequency_data* svc(void**) override {
            auto segment_size = compute_segment_size(text, workers);
            for(size_t i = 0; i < workers; i++) {
                auto pair = extract_task_range(text, segment_size, workers, i);
                ff_send_out_to(new frequency_data(pair.first, pair.second, i), i);
            }
            return EOS;
        }
    };

    frequency_output* extract_frequencies_ff_worker(frequency_data* data, ff_node*) {
        auto result = new frequency_output(
            extract_frequencies(data->text_start, data->text_end),
            data->worker
        );

        delete data;
        return result;
    }
    
    struct frequencyExtractionCollector: ff_minode_t<frequency_output, void*>
    {
    private:
        size_t workers;
        std::unordered_map<char, int>& total_frequencies;
        std::vector<std::unordered_map<char, int>>& frequencies;

    public:
        frequencyExtractionCollector(size_t workers, std::unordered_map<char, int>& total_frequencies, 
            std::vector<std::unordered_map<char, int>>& frequencies)
            : workers(workers), total_frequencies(total_frequencies), frequencies(frequencies) {}

        void** svc(frequency_output* output) override {
            auto& new_frequencies = output->frequencies;
            auto index = output->worker;

            combine_frequencies(total_frequencies, new_frequencies);
            frequencies[index] = new_frequencies;

            delete output;
            return GO_ON;
        }
    };

    void extract_frequencies_ff(
        std::unordered_map<char, int>& total_frequencies,
        std::vector<std::unordered_map<char, int>>& frequencies,
        std::string const& text,
        size_t workers
    ) {
        frequencies.resize(workers);
        auto fun = std::function(&detail::extract_frequencies_ff_worker);
        auto farm = ff_Farm<detail::frequency_data, detail::frequency_output>(fun, workers);
        auto emitter = detail::frequencyExtractionEmitter(text, workers);
        auto collector = detail::frequencyExtractionCollector(workers, total_frequencies, frequencies);
        farm.add_emitter(emitter);
        farm.add_collector(collector);
        farm.run_and_wait_end();
    }

    //encoding farm
    struct encoder_data {
        std::string::const_iterator text_start;
        std::string::const_iterator text_end;
        size_t worker;
        byte offset;
    };

    struct encoder_output {
        std::vector<byte> data;
        size_t worker;
        byte offset;
    };
    
    struct encodingEmitter: ff_monode_t<void*, encoder_data>
    {
    private:
        std::string const& text;
        encoderTable const& table;
        std::vector<byte>& offsets;
        std::vector<std::unordered_map<char, int>> const& frequencies;
        size_t workers;

    public:
        encodingEmitter(std::string const& text, encoderTable const& table, std::vector<byte>& offsets, 
            std::vector<std::unordered_map<char, int>> const& frequencies, size_t workers)
            : text(text), table(table), offsets(offsets), frequencies(frequencies), workers(workers) {}

        encoder_data* svc(void**) override {
            compute_serialization_offsets(table, frequencies, offsets, workers);

            auto segment_size = compute_segment_size(text, workers);
            for(size_t i = 0; i < workers; i++) {
                auto pair = extract_task_range(text, segment_size, workers, i);
                ff_send_out_to(new encoder_data(pair.first, pair.second, i, offsets[i]), i);
            }

            return EOS;
        }
    };

    encoder_output* encode_text_ff_worker(const encoderTable& table, encoder_data* data, ff_node*) {
        auto result = new encoder_output(
            encode_text(table, data->text_start, data->text_end, data->offset),
            data->worker,
            data->offset
        );

        delete data;
        return result;
    }

    struct encodingCollector: ff_minode_t<encoder_output, void*>
    {
    private:
        size_t workers;
        std::vector<byte>& out_data;
        std::vector<encoder_output*> in_data;

    public:
        encodingCollector(size_t workers, std::vector<byte>& out_data)
            : workers(workers), out_data(out_data), in_data(workers) {}

        void** svc(encoder_output* data) override {
            in_data[data->worker] = data;
            return GO_ON;
        }

        void svc_end() override {
            for(size_t i = 0; i < workers; i++) {
                detail::append_text_parallel(out_data, in_data[i]->data, in_data[i]->offset);
                delete in_data[i];
            }
        }
    };

    void encode_text_ff(
        encoderTable const& table,
        std::vector<std::unordered_map<char, int>>& frequencies,
        std::vector<byte>& out_data,
        std::string const& text,
        size_t workers
    ) {
        std::vector<byte> offsets(workers);
        auto fun = std::function([table](detail::encoder_data* data, ff_node* n) {
            return detail::encode_text_ff_worker(table, data, n);
        });
        
        auto farm = ff_Farm<detail::encoder_data, detail::encoder_output>(fun, workers);
        auto emitter = detail::encodingEmitter(text, table, offsets, frequencies, workers);
        auto collector = detail::encodingCollector(workers, out_data);
        farm.add_emitter(emitter);
        farm.add_collector(collector);
        farm.run_and_wait_end();
    }
}

namespace huffman::encoder
{
    std::vector<byte> encode_parallel_ff(std::string text, size_t workers) {
#ifdef CHRONO_ENABLED
        auto& timing = TimingLogger::instance();
        auto& frequencies_timer = timing.newTimer("02.00 - Extracting letter frequencies from the text (parallel).");
#endif

        //extract frequencies of letters (parallelized)
        std::unordered_map<char, int> total_frequencies;
        std::vector<std::unordered_map<char, int>> frequencies;
        detail::extract_frequencies_ff(total_frequencies, frequencies, text, workers);

#ifdef CHRONO_ENABLED
        frequencies_timer.stopTimer();
        auto& encodingTable_timer = timing.newTimer("02.01 - Building encoding table.");
#endif

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
        detail::encode_text_ff(table, frequencies, out_data, text, workers);

#ifdef CHRONO_ENABLED
        serialize_text_timer.stopTimer();
        serialization_timer.stopTimer();
#endif

        return out_data;
    }
}