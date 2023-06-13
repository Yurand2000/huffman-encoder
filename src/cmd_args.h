#ifndef COMMAND_LINE_ARGUMENTS_PARSER
#define COMMAND_LINE_ARGUMENTS_PARSER

#include <optional>
#include <string>

struct programOptions {
    bool encode;
    std::string input_file;
    std::string output_file;
    bool overwrite_output;
};

std::optional<programOptions> parse_arguments(int argc, char** argv);

#endif