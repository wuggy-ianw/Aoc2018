#include "file_parsing.h"

#include <fstream>
#include <sstream>
#include <cassert>
#include <regex>

// read a whole file into a plain string, newlines and all
std::string read_file(const std::string& filename)
{
    std::ifstream fs(filename);
    assert(fs);

    std::stringstream ss;
    ss << fs.rdbuf();

    return ss.str();
}

std::vector<std::string> split_string(const std::string& input, char delim)
{
    // see: https://stackoverflow.com/a/27511119

    std::stringstream ss(input);
    std::vector<std::string> split;
    std::string item;
    while(std::getline(ss, item, delim)) split.push_back(std::move(item));

    return split;
}

// split a string into separate lines
std::vector<std::string> parse_lines(const std::string& s)
{
    return split_string(s, '\n');
}
