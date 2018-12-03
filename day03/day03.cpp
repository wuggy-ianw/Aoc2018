#include <iostream>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <algorithm>

#include "../util/file_parsing.h"

struct claim
{
    int id;
    int x;
    int y;
    int width;
    int height;
};

struct claim parse_claim(const std::string& line)
{
    struct claim c = {};
    std::stringstream ss(line);

    ss >> '#' >> c.id;
    ss >> '@' >> c.x >> ',' >> c.y;
    ss >> ':' >> c.width >> 'x' >> c.height;

    assert(ss);
    return c;
}

std::pair<int, int> day03_solve_part1_and_2(const std::vector<struct claim>& claims)
{
    // find the maxes
    auto max_x_iter = std::max_element(claims.begin(), claims.end(), [](const struct claim& a, const struct claim& b) -> bool { return a.x + a.width < b.x + b.width; });
    size_t max_width = max_x_iter->x + max_x_iter->width + 1;

    auto max_y_iter = std::max_element(claims.begin(), claims.end(), [](const struct claim& a, const struct claim& b) -> bool { return a.y + a.height < b.y + b.height; });
    size_t max_height = max_y_iter->y + max_y_iter->height + 1;

    size_t max_id = std::max_element(claims.begin(), claims.end(), [](const struct claim& a, const struct claim& b) -> bool { return a.id < b.id; })->id;

    struct tile
    {
        int id = 0;
        int count = 0;
    };

    // make a grid
    std::vector<std::vector<tile>> grid;
    grid.resize(max_height);
    for (auto& row : grid) row.resize(max_width, {});

    std::vector<bool> overlapped;
    overlapped.resize(max_id, false);

    // count on all the grid squares
    int overlapping = 0;
    for (auto& c : claims)
    {
        for (size_t y = 0; y < c.height; ++y)
        {
            for (size_t x = 0; x < c.width; ++x)
            {
                auto& t = grid[c.y + y][c.x + x];

                // increase the overlap count
                ++t.count;
                if (t.count == 2) ++overlapping; // only count overlaps ONCE

                // set the id if this is the first time claiming this tile
                if (t.id)
                {
                    // this overlaps with something else
                    // set both ourselves and the original as overlapped
                    overlapped[t.id] = true;
                    overlapped[c.id] = true;
                }
                else
                {
                    t.id = c.id;    // we're the first to use this tile
                }
            }
        }
    }

    // search for the first non-overlapping id
    for (auto& c : claims)
    {
        if (!overlapped[c.id]) return {overlapping, c.id};
    }

    assert(false);  // no none overlapping region found?
    return {};
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty() > 0);

    auto claims = convert_lines<struct claim>(lines, parse_claim);

    auto result = day03_solve_part1_and_2(claims);
    std::cout << result.first << std::endl;
    std::cout << result.second << std::endl;
    return 0;
}

