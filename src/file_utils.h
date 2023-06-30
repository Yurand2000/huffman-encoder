#ifndef FILE_UTILS
#define FILE_UTILS

#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

std::string read_text_file(const std::string& filename);
std::vector<unsigned char> read_binary_file(const std::string& filename);
bool file_exists(const std::string& filename);

#endif