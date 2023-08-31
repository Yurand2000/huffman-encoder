#include "file_utils.h"

#include <string>
#include <filesystem>

std::string read_text_file(const std::string& filename)
{    
    auto file = std::ifstream(filename, std::ios::in | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("File \"" + filename + "\" does not exist.");
    }

    auto file_size = std::filesystem::file_size(filename);
    std::string out(file_size, '\0');
    file.read(out.data(), file_size);

    return out;
}

std::vector<unsigned char> read_binary_file(const std::string& filename)
{
    auto file = std::ifstream(filename, std::ios::binary);
    file.exceptions(std::ios_base::badbit);

    if (!file.is_open()) {
        throw std::runtime_error("File \"" + filename + "\" does not exist.");
    }

    auto file_size = std::filesystem::file_size(filename);
    if (file_size == 0) {
        return std::vector<unsigned char>{};
    }

    std::vector<unsigned char> result(file_size);
    file.read(reinterpret_cast<char*>(result.data()), file_size);

    return result;
}

bool file_exists(const std::string& filename) {
    auto file = std::ifstream(filename, std::ios::binary);
    return file.is_open();
}