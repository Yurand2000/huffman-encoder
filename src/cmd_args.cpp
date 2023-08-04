#include "cmd_args.h"

#include "file_utils.h"

inline void print_help() {
    std::cout << "Usage: (--encode | --decode) <input file> <output file> [-p <number of threads> [--ff]] [--overwrite]\n";
}

inline std::optional<programOptions> print_error(std::string message) {
    std::cout << message;
    print_help();
    return std::optional<programOptions>();
}

std::optional<programOptions> parse_arguments(int argc, char** argv)
{
    if (argc < 4 || argc > 8) {
        print_help();
        return std::optional<programOptions>();
    } else {
        auto options = programOptions();

        auto encode_str = std::string(argv[1]);
        options.input_file = std::string(argv[2]);
        options.output_file = std::string(argv[3]);

        int number_of_threads = -1;
        auto ff_str = std::string();
        auto overwrite_str = std::string();
        if (argc > 4) {
            auto arg4 = std::string(argv[4]);
            if (arg4 == "-p" && argc > 5) {
                number_of_threads = std::atoi(argv[5]);

                if (argc == 7) {
                    auto arg6 = std::string(argv[6]);
                    if (arg6 == "--ff") ff_str = arg6;
                    else overwrite_str = arg6;
                } else if (argc == 8) {
                    ff_str = std::string(argv[6]);
                    overwrite_str = std::string(argv[7]);
                }
            } else {
                overwrite_str = arg4;
            }
        }

        bool encode = false;
        if (encode_str == "--encode") {
            encode = true;
        } else if (encode_str == "--decode") {
            encode = false;
            options.encode = programMode::decode;
        } else {
            return print_error("Error, unrecognized command.\n");
        }

        if (!file_exists(options.input_file))
            return print_error("Error, specified input file does not exist.\n");

        options.overwrite_output = false;
        if (overwrite_str == "--overwrite")
            options.overwrite_output = true;
        else if (overwrite_str.size() != 0)
            return print_error("Error, unrecognized command.\n");
        
        if (file_exists(options.output_file) && !options.overwrite_output)
            return print_error("Error, specified output file already exists and would not be overwritten.\nSet the --overwrite flag to force overwrite.\n");

        if (encode) {
            if (number_of_threads == -1) {
                options.encode = programMode::encode;
            } else if (number_of_threads < 1) {
                return print_error("Error, unrecognized command.\n");
            } else if (ff_str.empty()) {
                options.number_of_workers = number_of_threads;
                options.encode = programMode::encodeParallelNative;
            } else if (ff_str != "--ff") {
                return print_error("Error, unrecognized command.\n");
            } else {
                options.number_of_workers = number_of_threads;
                options.encode = programMode::encodeParallelFastFlow;
            }
        }

        return std::optional(options);
    }
}