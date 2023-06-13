#include "cmd_args.h"

#include "file_utils.h"

inline void print_help() {
    std::printf("Usage: (--encode | --decode) <input file> <output file> [--overwrite]\n");
}

std::optional<programOptions> parse_arguments(int argc, char** argv)
{

    if (argc != 4 && argc != 5) {
        print_help();
        return std::optional<programOptions>();
    } else {
        auto options = programOptions();

        auto encode_str = std::string(argv[1]);
        options.input_file = std::string(argv[2]);
        options.output_file = std::string(argv[3]);
        auto overwrite_str = argc == 5 ? std::string(argv[4]) : std::string();

        bool encode = false;
        if (encode_str == "--encode") {
            options.encode = true;
        } else if (encode_str == "--decode") {
            options.encode = false;
        } else {
            std::printf("Error, unrecognized command.\n");
            print_help();
            return std::optional<programOptions>();
        }

        if (!file_exists(options.input_file)) {
            std::printf("Error, specified input file does not exist.\n");
            print_help();
            return std::optional<programOptions>();
        }

        options.overwrite_output = false;
        if (overwrite_str == "--overwrite") {
            options.overwrite_output = true;
        } else if (overwrite_str.size() != 0) {
            std::printf("Error, unrecognized command.\n");
            print_help();
            return std::optional<programOptions>();
        }
        
        if (file_exists(options.output_file) && !options.overwrite_output) {
            std::printf("Error, specified output file already exists and would not be overwritten.\n");
            std::printf("Set the --overwrite flag to force overwrite.\n");
            print_help();
            return std::optional<programOptions>();
        }

        return std::optional(options);
    }
}