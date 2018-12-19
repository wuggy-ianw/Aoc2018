#include <cassert>
#include <iostream>

#include "../util/file_parsing.h"

int day00_solve_part1()
{
    return 0;
}

int day00_solve_part2()
{
    return 0;
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    std::cout << day00_solve_part1() << std::endl;
    std::cout << day00_solve_part2() << std::endl;
    return 0;
}

