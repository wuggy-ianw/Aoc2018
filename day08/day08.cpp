#include <cassert>
#include <iostream>
#include <functional>

#include "../util/file_parsing.h"


using Iter = std::vector<int>::const_iterator;

std::pair<Iter, int> sum_metadata(Iter left, Iter limit)
{
    assert(left < limit);

    // extract the data for this node
    int n_children = *left++;
    int n_metadata = *left++;

    // left is now at the first child, get it's span and metadata
    int metadata = 0;
    for(int i=0; i < n_children; i++)
    {
        auto child = sum_metadata(left, limit);
        left = child.first;
        metadata += child.second;
    }

    // now add our metadata
    for(int i=0; i<n_metadata; i++)
    {
        metadata += *left++;
    }

    return {left, metadata};
}

std::pair<Iter, int> compute_value(Iter left, Iter limit)
{
    assert(left < limit);

    // extract the data for this node
    int n_children = *left++;
    int n_metadata = *left++;

    // if we have no children, then our value is the sum of our metadata
    if (n_children == 0)
    {
        int value = 0;
        for(int i=0; i < n_metadata; i++)
        {
            value += *left++;
        }

        return {left, value};
    }

    // otherwise we need to add the values of
    // our indexed children...
    std::vector<int> child_values;
    child_values.push_back(0);  // the 'zero-index' child has a value of zero

    for(int i=0; i < n_children; i++)
    {
        auto child = compute_value(left, limit);
        left = child.first;
        child_values.push_back(child.second);
    }

    // now index our childs values with our metadata
    int value = 0;
    for(int i=0; i<n_metadata; i++)
    {
        int index = *left++;
        if (index < 0 || index >=child_values.size()) continue; // skip out of range indices
        else value += child_values[index];
    }

    return {left, value};
}


int day08_solve_part1(const std::vector<int>& numbers)
{
    auto root = sum_metadata(numbers.begin(), numbers.end());
    assert(root.first == numbers.end());    // root should span all the numbers
    return root.second;
}

int day08_solve_part2(const std::vector<int>& numbers)
{
    auto root = compute_value(numbers.begin(), numbers.end());
    assert(root.first == numbers.end());    // root should span all the numbers
    return root.second;
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(lines.size() == 1);  // expect exactly 1 line of input

    auto fields = split_string(lines[0], ' ');
    assert(!fields.empty());

    std::vector<int> numbers = convert_strings<int>(fields, [](const std::string &s) -> int
    { return std::stoi(s); });

    std::cout << day08_solve_part1(numbers) << std::endl;
    std::cout << day08_solve_part2(numbers) << std::endl;
    return 0;
}

