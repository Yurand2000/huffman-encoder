#ifndef COMMAND_LINE_ARGUMENTS_PARSER
#define COMMAND_LINE_ARGUMENTS_PARSER

#include <optional>
#include <string>

enum programMode {
    encode,
    decode,
    encodeParallelNative,
    encodeParallelFastFlow
};

struct programOptions {
    programMode encode;
    size_t number_of_workers;
    std::string input_file;
    std::string output_file;
    bool overwrite_output;
};

std::optional<programOptions> parse_arguments(int argc, char** argv);

#endif