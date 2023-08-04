#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>

#include "file_utils.h"
#include "cmd_args.h"

#include "encoder/encoder.h"
#include "decoder/decoder.h"

using namespace huffman;

int main(int argc, char** argv)
{
    auto programOptions = parse_arguments(argc, argv);
    if (!programOptions.has_value()) {
        return 1;
    }

    auto options = programOptions.value();    
    if (file_exists(options.output_file) && !options.overwrite_output) {
        printf("Output file %s already exists. You may force overwrite if you wish.", options.output_file.c_str());
        return 1;
    }

    if (options.encode == programMode::decode) {
        auto encoded_text = read_binary_file(options.input_file);
        auto text = decoder::decode(encoded_text);

        auto file = std::ofstream(options.output_file);
        file << text << std::flush;        
    } else {
        auto text = read_text_file(options.input_file);

        std::vector<unsigned char> encoded_text;
        switch (options.encode) {
            default:
            case programMode::encode:
                encoded_text = encoder::encode(text);
                break;
            case programMode::encodeParallelNative:
                encoded_text = encoder::encode_parallel_native(text, options.number_of_workers);
                break;
            case programMode::encodeParallelFastFlow:
                encoded_text = encoder::encode_parallel_ff(text, options.number_of_workers);
                break;
        }
        
        auto file = std::ofstream(options.output_file);
        file.write(reinterpret_cast<char*>(encoded_text.data()), encoded_text.size());
        file.flush();
    }

    return 0;
}