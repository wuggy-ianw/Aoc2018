#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include "../util/file_parsing.h"
#include "../util/grid.h"



using Grid = NoddySparseGrid<char>;

std::pair<int, int> process_ordinates(Grid& g, const std::vector<std::string>& ordinates)
{
    int miny = std::numeric_limits<int>::max();
    int maxy = std::numeric_limits<int>::min();

    for (auto& l : ordinates)
    {
        std::stringstream ss(l);

        char first_ord, second_ord;
        int first_val, second_val, third_val;

        ss >> first_ord >> '=' >> first_val >> ','
           >> second_ord >> '=' >> second_val >> ".." >> third_val;

        assert(ss);
        assert(first_ord == 'x' || first_ord == 'y');
        if (first_ord == 'x')
        {
            assert(second_ord == 'y');
            int x = first_val;
            for (int y = second_val; y<=third_val; ++y)
            {
                g[y][x] = '#';
                miny = std::min(miny, y);
                maxy = std::max(maxy, y);
            }
        }
        else
        {
            assert(second_ord == 'x');
            int y = first_val;
            miny = std::min(miny, y);
            maxy = std::max(maxy, y);
            for (int x = second_val; x<=third_val; ++x) g[y][x] = '#';
        }
    }

    return {miny, maxy};
}


void print_grid(std::ostream& os, const Grid& g)
{
    for(size_t r = 0; r < g.rows(); ++r)
    {
        for(size_t c = 0; c < g.columns(); ++c) os << g[r][c];
        os << std::endl;
    }
}


struct flow_result
{
    int wet_tiles = 0;
    int retained_tiles = 0;
    bool blocked = false;

    flow_result& operator+=(const flow_result& o)
    {
        wet_tiles += o.wet_tiles;
        retained_tiles += o.retained_tiles;

        // don't propagate blocking on addition
    }
};



flow_result flow(const std::pair<int, int>& ybounds, Grid& grid, Point p, bool ignore_left = false, bool ignore_right = false)
{
    flow_result result{};

    // if we're off the maximum y value, then this is a non-blocked, zero wet-tiles end-state
    if (p.y > ybounds.second) return result;

    // if we're a blocking tile (water or clay) then this is a blocked, zero-wet-tiles end-state
    // (if we're water, then we're already counted)
    if (grid[p] == '#' || grid[p] == '~')
    {
        result.blocked = true;
        return result;
    }

    // if we're already a flow tile... then ... what?
    if (grid[p] == '|')
    {
        int x = 0;
        return result;  // not-blocked, no extra count?
    }

    // This MUST be a wet tile. If it's within bounds, count it
    grid[p] = '|';
    if (p.y >= ybounds.first && p.y <= ybounds.second) ++result.wet_tiles;

    // check below
    Point below{p.x, p.y + 1};
    auto result_below = flow(ybounds, grid, below);
    result += result_below;

    // if below is not blocking, then we're not blocking, sum our wet tiles
    if (!result_below.blocked)
    {
        result.blocked = false;
        return result;
    }
    // below is blocking, so water flows sideways
    // check left and right
    Point left{p.x - 1, p.y};
    Point right{p.x + 1, p.y};

    flow_result result_left{};
    if (!ignore_left) result_left = flow(ybounds, grid, left, false, true);

    flow_result result_right;
    if (!ignore_right) result_right = flow(ybounds, grid, right, true, false);

    // if we're ignoring left or right, then use the blocking result of the not-ignored direction
    if (ignore_left) result_left.blocked = result_right.blocked;
    if (ignore_right) result_right.blocked = result_left.blocked;

    // if left and right are both blocking, then we're blocking!
    // otherwise, we're not blocking
    // either way, sum our local, left and right counts
    result.blocked = result_left.blocked && result_right.blocked;
    result += result_left;
    result += result_right;

    // if we're blocking, use the full-of-water symbol
    if ((!ignore_left && !ignore_right) && result.blocked)
    {
        // fill this tile - we're not flowing
        grid[p] = '~';
        ++result.retained_tiles;

        // also left and right of here that are flow characters are filling
        for (int x = p.x + 1; grid[p.y][x] == '|'; ++x) { grid[p.y][x] = '~'; ++result.retained_tiles; }
        for (int x = p.x - 1; grid[p.y][x] == '|'; --x) { grid[p.y][x] = '~'; ++result.retained_tiles; }
    }

    return result;
}

flow_result day17_solve_part1_and_2(const std::pair<int, int>& ybounds, const Grid& initial_grid)
{
    const Point spring{500, ybounds.first - 1};
    Grid grid = initial_grid;

    return flow(ybounds, grid, spring);
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    Grid grid('.');
    auto ybounds = process_ordinates(grid, lines);

    auto result = day17_solve_part1_and_2(ybounds, grid);

    std::cout << result.wet_tiles << std::endl;
    std::cout << result.retained_tiles << std::endl;
    return 0;
}

