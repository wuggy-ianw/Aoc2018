#include <cassert>
#include <iostream>

#include "../util/file_parsing.h"
#include "../util/grid.h"


void populate_grid(NoddySparseGrid<char>& g, const std::vector<std::string>& lines)
{
    for (size_t y = 0; y<lines.size(); ++y)
    {
        const auto& l = lines[y];
        for (size_t x = 0; x<l.size(); ++x)
        {
            g[y][x] = l[x];
        }
    }
}

int adjacent_count(char c, const NoddySparseGrid<char>& g, size_t ox, size_t oy)
{
    size_t minx = ox >= 1 ? ox - 1 : ox;
    size_t miny = oy >= 1 ? oy - 1 : oy;

    int count = 0;
    for (size_t y = miny ; y <= oy + 1; ++y)
    {
        for (size_t x = minx; x <= ox + 1; ++x)
        {
            if (x==ox && y==oy) continue;   // skip center tile
            if (g[y][x] == c) ++count;
        }
    }

    return count;
}

char update_state(const NoddySparseGrid<char>& g, size_t x, size_t y)
{
    char c = g[y][x];
    if (c == '.')
    {
        return adjacent_count('|', g, x, y) >= 3 ? '|' : '.';
    }
    else if (c == '|')
    {
        return adjacent_count('#', g, x, y) >= 3 ? '#' : '|';
    }
    else if (c == '#')
    {
        return (adjacent_count('#', g, x, y)>=1 && adjacent_count('|', g, x, y)>=1)? '#' : '.';
    }
    else assert(false); // unknown state?!
}

void print_grid(std::ostream& os, const NoddySparseGrid<char>& g)
{
    for(size_t r = 0; r < g.rows(); ++r)
    {
        for(size_t c = 0; c < g.columns(); ++c) os << g[r][c];
        os << std::endl;
    }
}

size_t run_one_step(const NoddySparseGrid<char>& sg, NoddySparseGrid<char>& dg)
{
    size_t wooded = 0;
    size_t lumber_yards = 0;

    for (size_t r = 0; r < sg.rows(); ++r)
    {
        for (size_t c = 0; c < sg.columns(); ++c)
        {
            char t = update_state(sg, c, r);
            dg[r][c] = t;

            if (t=='|') ++wooded;
            if (t=='#') ++lumber_yards;
        }
    }

    return wooded * lumber_yards;
}



size_t day18_solve_part1(size_t iterations, const NoddySparseGrid<char>& initial_grid)
{
    NoddySparseGrid<char> grid1 = initial_grid;
    NoddySparseGrid<char> grid2;

    NoddySparseGrid<char>* source_grid_ptr = &grid1;
    NoddySparseGrid<char>* dest_grid_ptr = &grid2;

    size_t resource_value = 0;

    for(size_t i = 0; i < iterations; i++)
    {

        const NoddySparseGrid<char>& sg = *source_grid_ptr;
        NoddySparseGrid<char>& dg = *dest_grid_ptr;

        resource_value = run_one_step(sg, dg);
        std::swap(source_grid_ptr, dest_grid_ptr);
    }

    return resource_value;
}

size_t day18_solve_part2(const NoddySparseGrid<char>& initial_grid)
{
    constexpr size_t end_iteration = 1000000000ull;
    constexpr size_t skip_start = 1000ull;

    NoddySparseGrid<char> grid1 = initial_grid;
    NoddySparseGrid<char> grid2;

    NoddySparseGrid<char>* source_grid_ptr = &grid1;
    NoddySparseGrid<char>* dest_grid_ptr = &grid2;

    // run until we get a 'cycle' based on the resource value...
    // this might give false +ives... but we might be lucky
    std::unordered_map<size_t, size_t> resource_val_iteration;
    std::unordered_map<size_t, size_t> iteration_resource_val;

    auto found_cycle = resource_val_iteration.end();

    size_t i = 0;
    while(found_cycle == resource_val_iteration.end())
    {
        const NoddySparseGrid<char>& sg = *source_grid_ptr;
        NoddySparseGrid<char>& dg = *dest_grid_ptr;

        size_t resource_value = run_one_step(sg, dg);

        if (i >= skip_start) {
            found_cycle = resource_val_iteration.find(resource_value);
            if (found_cycle == resource_val_iteration.end()) {
                resource_val_iteration[resource_value] = i;
                iteration_resource_val[i] = resource_value;
            }
        }
        std::swap(source_grid_ptr, dest_grid_ptr);
        i++;
    }

    // where in the cycle
    size_t cycle_start = found_cycle->second;
    size_t cycle_length = i - cycle_start - 1;

    size_t cycle_pos = (end_iteration - cycle_start - 1) % cycle_length;
    size_t resource_value = iteration_resource_val[cycle_start + cycle_pos];
    return resource_value;
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    NoddySparseGrid<char> grid('.');
    populate_grid(grid, lines);

    std::cout << day18_solve_part1(10, grid) << std::endl;
    std::cout << day18_solve_part2(grid) << std::endl;
    return 0;
}

