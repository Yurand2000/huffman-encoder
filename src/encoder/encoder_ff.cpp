#include "encoder.h"

#include <ff/ff.hpp>

#include "encoder_table.h"
#include "character_serializer.h"
#include "../utils.h"

#include "../threads/threadTask.h"

using namespace ff;

namespace huffman::encoder::detail
{
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
            auto segment_size = positive_div_ceil(text.length(), workers);
            for(size_t i = 0; i < workers; i++) {
                auto begin = text.cbegin() + segment_size * i;
                auto end = (i == workers - 1) ? text.cend() : begin + segment_size;
                ff_send_out_to(new frequency_data(begin, end, i), i);
            }
            return EOS;
        }
    };

    std::unordered_map<char, int> extract_frequencies(std::string::const_iterator, std::string::const_iterator);
    
    frequency_output* extract_frequencies_ff(frequency_data* data, ff_node*) {
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

            for(auto pair : new_frequencies) {
                if (total_frequencies.find(pair.first) != total_frequencies.end())
                    total_frequencies[pair.first] += pair.second;
                else
                    total_frequencies.emplace(pair.first, pair.second);
            }
            frequencies[index] = new_frequencies;

            delete output;
            return GO_ON;
        }
    };

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
    
    size_t count_bits(const encoderTable&, std::unordered_map<char, int>);
    
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
            auto segment_size = positive_div_ceil(text.length(), workers);
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
                ff_send_out_to(new encoder_data(begin, end, i, offsets[i]), i);
            }
            return EOS;
        }
    };

    std::vector<byte> encode_text(const encoderTable&, std::string::const_iterator, std::string::const_iterator, byte);

    encoder_output* encode_text_ff(const encoderTable& table, encoder_data* data, ff_node*) {
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
                auto& data = in_data[i]->data;
                auto& offset = in_data[i]->offset;

                if (!data.empty()) {
                    if (offset == 0) {
                        out_data.insert(out_data.end(), data.begin(), data.end());
                    } else {
                        out_data.back() |= data[0];
                        out_data.insert(out_data.end(), data.begin() + 1, data.end());
                    }
                }   

                delete in_data[i];
            }
        }
    };
}

namespace huffman::encoder
{
    std::vector<byte> encode_parallel_ff(std::string text, size_t workers) {
        std::unordered_map<char, int> total_frequencies {};
        std::vector<std::unordered_map<char, int>> frequencies(workers);
        auto fun = std::function(&detail::extract_frequencies_ff);
        auto farm = ff_Farm<detail::frequency_data, detail::frequency_output>(fun, workers);
        auto emitter = detail::frequencyExtractionEmitter(text, workers);
        auto collector = detail::frequencyExtractionCollector(workers, total_frequencies, frequencies);
        farm.add_emitter(emitter);
        farm.add_collector(collector);
        farm.run_and_wait_end();

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
        auto fun2 = std::function([table](detail::encoder_data* data, ff_node* n) {
            return detail::encode_text_ff(table, data, n);
        });
        std::vector<byte> offsets(workers);
        auto farm2 = ff_Farm<detail::encoder_data, detail::encoder_output>(fun2, workers);
        auto emitter2 = detail::encodingEmitter(text, table, offsets, frequencies, workers);
        auto collector2 = detail::encodingCollector(workers, out_data);
        farm2.add_emitter(emitter2);
        farm2.add_collector(collector2);
        farm2.run_and_wait_end();

        return out_data;
    }
}