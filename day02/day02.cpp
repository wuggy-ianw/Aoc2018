#include <iostream>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <sstream>

#include "../util/file_parsing.h"



int day02_solve_part1(const std::vector<std::string>& lines)
{
    int n_pairs = 0;
    int n_triples = 0;

    for (auto& l : lines)
    {
        std::unordered_map<char, int> counted;
        for (auto c : l) counted[c]++;

        bool has_pair = false;
        bool has_triple = false;
        for( auto& pair : counted)
        {
            has_pair |= (pair.second == 2);
            has_triple |= (pair.second == 3);
        }

        n_pairs += has_pair ? 1 : 0;
        n_triples += has_triple ? 1 : 0;
    }

    return n_pairs * n_triples;
}

std::string day02_solve_part2(const std::vector<std::string>& lines)
{
    // find the pair of lines that differ by only one character
    for (auto x = lines.begin(); x != lines.end(); x++)
    {
        for (auto y = x + 1; y != lines.end(); ++y)
        {
            int diffcount = 0;
            if (x->size() == y->size())
            {
                for(size_t i = 0; i < x->size(); ++i)
                {
                    diffcount += ((*x)[i] == (*y)[i] ? 0 : 1);
                    if (diffcount > 1) break;
                }
            }

            if (diffcount == 1)
            {
                // strings x and y only differ by 1 character!
                std::stringstream ss;
                for (size_t i = 0; i < x->size(); ++i)
                {
                    if ((*x)[i] == (*y)[i]) ss << (*x)[i];
                }

                return ss.str();
            }
        }
    }

    assert(false);      // didn't find a solution?
    return "";
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty() > 0);

    std::cout << day02_solve_part1(lines) << std::endl;
    std::cout << day02_solve_part2(lines) << std::endl;
    return 0;
}

