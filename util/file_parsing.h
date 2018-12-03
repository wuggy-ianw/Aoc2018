//
// Created by ian.woods on 01/12/18.
//

#ifndef AOC2018_FILE_PARSING_H
#define AOC2018_FILE_PARSING_H

#include <vector>
#include <string>
#include <functional>
#include <iostream>

// read a whole file into a plain string, newlines and all
std::string read_file(const std::string& filename);

// split a string into separate lines
std::vector<std::string> parse_lines(const std::string& s);

// convert a pile of strings into a pile of... something else...
template<typename T>
std::vector<T> convert_lines(const std::vector<std::string>& s, const std::function<T(const std::string&)>& c)
{
    std::vector<T> converted;
    for (const auto& line : s) converted.emplace_back(c(line));
    return converted;
}



template<class e, class t>
std::basic_istream<e,t>& operator >>(std::basic_istream<e,t>& in, const e& cliteral)
{
    e buffer(0);
    in >> buffer;
    if (buffer != cliteral) in.setstate(std::ios::failbit);
    return in;
}

#endif //AOC2018_FILE_PARSING_H
