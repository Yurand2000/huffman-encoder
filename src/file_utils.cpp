#include "file_utils.h"

std::string read_text_file(const std::string& filename)
{
    constexpr auto read_size = std::size_t(4096);
    auto file = std::ifstream(filename);
    file.exceptions(std::ios_base::badbit);

    if (!file.is_open()) {
        throw std::runtime_error("File \"" + filename + "\" does not exist.");
    }

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (file.read(&buf[0], read_size)) {
        out.append(buf, 0, file.gcount());
    }
    out.append(buf, 0, file.gcount());
    return out;
}

std::vector<unsigned char> read_binary_file(const std::string& filename)
{
    auto file = std::ifstream(filename, std::ios::binary);
    file.exceptions(std::ios_base::badbit);

    if (!file.is_open()) {
        throw std::runtime_error("File \"" + filename + "\" does not exist.");
    }

    file.seekg(0, std::ios::end);
    std::ifstream::pos_type size = file.tellg();

    if (size == 0) {
        return std::vector<unsigned char>{};
    }

    std::vector<unsigned char> result(size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(result.data()), size);

    return result;
}

bool file_exists(const std::string& filename) {
    auto file = std::ifstream(filename, std::ios::binary);
    return file.is_open();
}