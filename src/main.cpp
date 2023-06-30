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

    if (options.encode) {
        auto text = read_text_file(options.input_file);
        auto encoded_text = encoder::encode(text);
        
        auto file = std::ofstream(options.output_file);
        file.write(reinterpret_cast<char*>(encoded_text.data()), encoded_text.size());
        file.flush();
    } else {
        auto encoded_text = read_binary_file(options.input_file);
        auto text = decoder::decode(encoded_text);

        auto file = std::ofstream(options.output_file);
        file << text << std::flush;
    }

    return 0;
}