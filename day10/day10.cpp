#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>

#include "../util/file_parsing.h"


struct particle
{
    int x = 0;
    int y = 0;

    int dx = 0;
    int dy = 0;
};

struct particle parse_particle(const std::string& s)
{
    struct particle p;

    std::stringstream ss(s);
    ss >> "position=<" >> p.x >> ',' >> p.y >> '>' >> "velocity=<" >> p.dx >> ',' >> p.dy >> '>';
    assert(ss);
    return p;
}


int compute_score(const std::vector<struct particle>& particles_t0, int time)
{
    int min_x = std::numeric_limits<int>::max();
    int min_y = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int max_y = std::numeric_limits<int>::min();

    for (const auto& p : particles_t0)
    {
        int xt = p.x + p.dx * time;
        int yt = p.y + p.dy * time;

        min_x = std::min(min_x, xt);
        max_x = std::max(max_x, xt);
        min_y = std::min(min_y, yt);
        max_y = std::max(max_y, yt);
    }

    return (max_x - min_x) + (max_y - min_y);
}

std::string make_grid(const std::vector<struct particle>& particles_t0, int time)
{
    std::vector<std::vector<char>> grid;

    for (const auto& p : particles_t0)
    {
        int xt = p.x + p.dx * time;
        int yt = p.y + p.dy * time;

        // assume that we only get +ive positions in the solution!
        assert(xt >= 0);
        assert(yt >= 0);

        // set the point in the grid for this position
        if (grid.size() < yt) grid.resize(yt+1);
        if (grid[yt].size() < xt) grid[yt].resize(xt+1, ' ');
        grid[yt][xt] = '#';
    }

    std::stringstream ss;
    for (const auto& row : grid)
    {
        // skip empty lines
        if (row.empty()) continue;

        for (const auto& c : row)
        {
            ss << c;
        }
        ss << std::endl;
    }

    return ss.str();
}

std::pair<std::string, int> day10_solve_part1_and_2(const std::vector<struct particle>& particles_t0)
{
    // Find the 't' that minimises the bounds
    // that's very likely to be 'near' the solution
    // The score for the sum of the x, y bounds should be mostly monotonic

    // use a simple binary search
    int left_bound = 0;
    int right_bound = 65536;

    int score_left = compute_score(particles_t0, left_bound);
    int score_right = compute_score(particles_t0, right_bound);
    while (left_bound < right_bound)
    {
        int mid_point = (left_bound + right_bound) >> 1;
        if (mid_point == left_bound || mid_point == right_bound)
        {
            // if we're down to left or right, pick the lower
            assert(right_bound - left_bound < 2);

            // we happen to have already picked a zero-width bound... use left
            if (left_bound == right_bound) break;

            // left is better than right... use left
            if (score_left < score_right) break;

            // right is better than left... we're going to use left, so make it the same as right
            left_bound = right_bound;
            score_left = score_right;
            break;
        }

        int score_mid = compute_score(particles_t0, mid_point);

        // take the two smallest scores as the new left and right bounds
        if (score_left + score_mid < score_mid + score_right)
        {
            // left to mid is lower
            right_bound = mid_point;
            score_right = score_mid;
        } else
        {
            // mid to right is lower
            left_bound = mid_point;
            score_left = score_mid;
        }
    }

   return {make_grid(particles_t0, left_bound), left_bound};
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    auto particles_t0 = convert_strings<struct particle>(lines, parse_particle);
    assert(!particles_t0.empty());

    auto result = day10_solve_part1_and_2(particles_t0);
    std::cout << result.first << std::endl;
    std::cout << result.second << std::endl;
    return 0;
}

