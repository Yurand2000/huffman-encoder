#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>

#include "file_utils.h"
#include "cmd_args.h"

int main(int argc, char** argv)
{
    auto programOptions = parse_arguments(argc, argv);
    if (!programOptions.has_value()) {
        return 1;
    }

    auto options = programOptions.value();    

    if (options.encode) {
        printf("%s\n", read_text_file(options.input_file).c_str());
    } else {
        printf("Not implemented.\n");
    }

    return 0;
}