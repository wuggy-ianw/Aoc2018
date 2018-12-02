#include <iostream>
#include <cassert>
#include <functional>
#include <numeric>
#include <unordered_set>

#include "../util/file_parsing.h"

int day01_solve_part1(const std::vector<int>& numbers)
{
    return std::accumulate(numbers.begin(), numbers.end(), 0);
}

int day01_solve_part2(const std::vector<int>& numbers)
{
    std::unordered_set<int> freq_set;
    int freq = 0;
    while(true)
    {
        for (auto i : numbers)
        {
            freq += i;
            auto insert_pair = freq_set.insert(freq);
            if (!insert_pair.second) return freq; // insert failed because the entry already exists
        }
    }
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty() > 0);

    std::vector<int> numbers = convert_lines<int>(lines, [](const std::string& s) -> int { return std::stoi(s); });

    std::cout << day01_solve_part1(numbers) << std::endl;
    std::cout << day01_solve_part2(numbers) << std::endl;
    return 0;
}

