#include <iostream>
#include <cassert>
#include <vector>
#include <regex>
#include <stack>

#include "../util/file_parsing.h"

bool are_opposites(char a, char b)
{
    // only works in ascii!
    int d = a^' ';
    bool v = (d==b);
    return v;
}


size_t collapse_polymer(const std::string& polymers, char ignore)
{
    std::stack<char> p;

    for (char c : polymers)
    {
        if (ignore == tolower(c)) continue; // skip the ignore polymer


        if (p.empty()) p.push(c);                       // nothing to react with
        else if (are_opposites(c, p.top())) p.pop();    // did react with neighbour
        else p.push(c);                                 // didn't react
    }

    return p.size();
}

size_t day05_solve_part1(const std::string& polymers)
{
    return collapse_polymer(polymers, 0);
}

size_t day05_solve_part2(const std::string& polymers)
{
    // only works in ascii
    size_t smallest_size = polymers.size();
    char smallest_ignored = ' ';

    for(char c='a'; c <= 'z'; ++c)
    {
        size_t this_size = collapse_polymer(polymers, c);
        if (this_size < smallest_size)
        {
            smallest_size = this_size;
            smallest_ignored = c;
        }
    }

    return smallest_size;
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(lines.size() == 1);  // expect only 1 line!

    std::cout << day05_solve_part1(lines[0]) << std::endl;
    std::cout << day05_solve_part2(lines[0]) << std::endl;
    return 0;
}

