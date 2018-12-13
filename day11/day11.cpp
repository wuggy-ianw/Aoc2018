#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <tuple>

#include "tbb/blocked_range3d.h"
#include "tbb/parallel_reduce.h"


struct kernel
{
    static constexpr int width = 300;
    static constexpr int height = 300;

    static constexpr int left = 1;
    static constexpr int top = 1;

    static constexpr int right = left + width - 1;  // inclusive
    static constexpr int bottom = top + height - 1; // inclusive

    const int grid_serial;

    int best_power;
    int best_x;
    int best_y;
    int best_sq;

    explicit kernel(int grid_serial)
        : grid_serial(grid_serial),
          best_power(std::numeric_limits<int>::min()),
          best_x(-1),
          best_y(-1),
          best_sq(-1)
    {
    }

    kernel(kernel& o, tbb::split)
        : grid_serial(o.grid_serial),
          best_power(std::numeric_limits<int>::min()),
          best_x(-1),
          best_y(-1),
          best_sq(-1)
    {
    }

    int cell_power(int x, int y)
    {
        assert(x <= width);
        assert(y <= height);

        // Find the fuel cell's rack ID, which is its X coordinate plus 10.
        const long rack_id = x + 10;

        // Begin with a power level of the rack ID times the Y coordinate.
        const long power_level = rack_id * y;

        // Increase the power level by the value of the grid serial number (your puzzle input).
        const long power_level_a = power_level + grid_serial;

        // Set the power level to itself multiplied by the rack ID.
        const long power_level_b = power_level_a * rack_id;

        // Keep only the hundreds digit of the power level (so 12345 becomes 3; numbers with no hundreds digit become 0).
        const long hundreds = (power_level_b / 100) % 10;

        // Subtract 5 from the power level.
        const int power = hundreds - 5;
        assert(power < 5);
        assert(power > -6);

        return power;
    }

    void operator()(const tbb::blocked_range3d<int, int>& range)
    {
        for (int y = range.rows().begin() ; y < range.rows().end() ; ++y)
        {
            for (int x = range.cols().begin() ; x < range.cols().end() ; ++x)
            {
                // compute for a growing square from min_sq to max_sq
                // basically iterate up to max_sq in a pattern of the new outer bottom/right edge
                // only consider values between min_sq and max_sq (inclusive) to be 'valid'
                for (int square_size = range.pages().begin(); square_size < range.pages().end(); ++square_size)
                {
                    // if the square goes over the edge, we're done
                    if ((x+square_size > right) || (y+square_size > bottom)) break;

                    // This wastefully computes the squares each time...
                    // but since the computation is cheap, this is fine
                    // If we really cared, we could do something like DP and compute the best square
                    // by combining answers from each subsquare... but, well, this only takes a few seconds anyway
                    int square_power = 0;
                    for (int dy = 0; dy < square_size; ++dy)
                    {
                        for (int dx = 0; dx < square_size; ++dx)
                        {
                            square_power += cell_power(x + dx, y + dy);
                        }
                    }

                    // if this square's power is best, keep it
                    if (square_power > best_power)
                    {
                        best_power = square_power;
                        best_x = x;
                        best_y = y;
                        best_sq = square_size;
                    }
                }

            }
        }
    }

    void join(kernel& o)
    {
        // pick the best power from us or the other kernel
        if (o.best_power > best_power)
        {
            best_power = o.best_power;
            best_x = o.best_x;
            best_y = o.best_y;
            best_sq = o.best_sq;
        }
    }
};



std::pair<int, int> day11_solve_part1(int grid_serial)
{

    constexpr int part1_square_size = 3;


    struct kernel k(grid_serial);
    tbb::parallel_reduce(
            tbb::blocked_range3d<int, int, int>(part1_square_size, part1_square_size + 1,
                                                k.top, k.bottom + 1,
                                                k.left,k.right + 1),
            k);

    return {k.best_x, k.best_y};
}

std::tuple<int, int, int> day11_solve_part2(int grid_serial)
{

    struct kernel k(grid_serial);
    tbb::parallel_reduce(
            tbb::blocked_range3d<int, int, int>(1, k.right + 1,
                                                k.top, k.bottom + 1,
                                                k.left,k.right + 1),
            k);

    return {k.best_x, k.best_y, k.best_sq};
}


int main()
{
    const int grid_serial = 8979;

    auto result_part1 = day11_solve_part1(grid_serial);
    std::cout << result_part1.first << "," << result_part1.second << std::endl;

    auto result_part2 = day11_solve_part2(grid_serial);
    std::cout << std::get<0>(result_part2) << "," << std::get<1>(result_part2) << "," << std::get<2>(result_part2) << std::endl;

    return 0;
}

