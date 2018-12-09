#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "../util/file_parsing.h"

#include "tbb/blocked_range2d.h"
#include "tbb/parallel_reduce.h"

struct point
{
    int x = 0;
    int y = 0;

    point() = default;
    point(int ax, int ay) : x(ax), y(ay) {}
};

struct point parse_point(const std::string& s)
{
    std::stringstream ss(s);
    struct point p;

    ss >> p.x >> ',' >> p.y;
    assert(ss);
    return p;
};

struct region
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    region() = default;
    region(int ax, int ay, int w, int h) : x(ax), y(ay), width(w), height(h)
    {
        assert(width > 0);
        assert(height > 0);
    }

    bool includes(const struct point& p) const
    {
        return (p.x >= x) && (p.y >= y) && (p.x < x + width) && (p.y < y + height);
    }

    bool excludes(const struct point& p) const
    {
        return (p.x < x) || (p.y < y) || (p.x >= x + width) || (p.y >= y + height);
    }
};


int dist(const point& a, const point& b)
{
    // manhattan distance
    return abs(a.x - b.x) + abs(a.y - b.y);
}

int nearest_point_index(const std::vector<struct point>& points, const point& p)
{
    // returns the index of the nearest point, or -1 if there is no nearest point (i.e. two nearest points are equidistant)
    int best_dist = dist(points[0], p);
    int best_index = 0;
    bool is_unique = true;  // if another dist is the same as best_dist, then this will be made false!

    for (int i=1; i<points.size(); ++i)
    {
        int d = dist(points[i], p);
        if (d < best_dist)
        {
            is_unique = true;
            best_dist = d;
            best_index = i;
        }
        else if (d == best_dist)
        {
            is_unique = false;
        }
    }

    return is_unique ? best_index : -1;
}

struct region find_bounds(const std::vector<struct point>& points)
{
    auto minmax_x = std::minmax_element(points.begin(), points.end(), [](const struct point& a, const struct point& b) -> bool { return a.x < b.x; });
    auto minmax_y = std::minmax_element(points.begin(), points.end(), [](const struct point& a, const struct point& b) -> bool { return a.y < b.y; });

    return {minmax_x.first->x,
            minmax_y.first->y,
            minmax_x.second->x - minmax_x.first->x + 1,
            minmax_y.second->y - minmax_y.first->y + 1};
};


int day06_solve_part1(const std::vector<struct point>& points)
{
    // find the bounds of the points with a smaller border
    auto inner_bound = find_bounds(points);
    struct region outer_bound = {inner_bound.x - 1, inner_bound.y - 1, inner_bound.width + 2, inner_bound.height + 2};

    struct kernel
    {
        std::vector<int> region_count;

        const std::vector<struct point>& points;
        const struct region& inner_bound;

        kernel(const std::vector<struct point>& p, const struct region& i) : points(p), inner_bound(i)
        {
            region_count.resize(points.size(), 0);
        }

        kernel(kernel& o, tbb::split) : points(o.points), inner_bound(o.inner_bound)
        {
            region_count.resize(points.size(), 0);
        }

        void operator()(const tbb::blocked_range2d<int, int>& range)
        {
            for (int y = range.rows().begin() ; y < range.rows().end() ; ++y)
            {
                for (int x = range.cols().begin() ; x < range.cols().end() ; ++x)
                {
                    struct point this_point(x, y);

                    //!! part 1 solver
                    // for this point, determine which is the nearest point in points
                    // if exactly between two points, discount it
                    // increment the region count for that nearest point (if any)
                    int nearest_index = nearest_point_index(points, this_point);
                    if (nearest_index < 0) continue;    // skip if no nearest

                    // increment the region count for the nearest...
                    // unless we've hit a boundary point, then mark it as 'infinite'
                    if (region_count[nearest_index] >= 0)
                    {
                        if (inner_bound.excludes(this_point)) region_count[nearest_index] = -1;
                        else ++region_count[nearest_index];
                    }
                }
            }
        }

        void join(kernel& o)
        {
            assert(o.region_count.size() == region_count.size());
            for (int i = 0; i < region_count.size(); ++i)
            {
                if (region_count[i] < 0 || o.region_count[i] < 0) region_count[i] = -1;
                else region_count[i] += o.region_count[i];
            }
        }

    };

    struct kernel k(points, inner_bound);
    tbb::parallel_reduce(
            tbb::blocked_range2d<int, int>(outer_bound.y, outer_bound.y + outer_bound.height, outer_bound.x, outer_bound.x + outer_bound.width),
            k);

    int largest_area = *std::max_element(k.region_count.begin(), k.region_count.end());
    return largest_area;
}

int day06_solve_part2(const std::vector<struct point>& points)
{
    // find the bounds of the points with a larger border!
    // this function will assert if the border wasn't big enough


    auto points_bound = find_bounds(points);
    int border = (2 * 10000) / static_cast<int>(points.size());   // estimate the maximum extent of the region
    struct region inner_bound = {inner_bound.x - border, inner_bound.y - border, inner_bound.width + (2 * border), inner_bound.height + (2 * border)};
    struct region outer_bound = {inner_bound.x - 1, inner_bound.y - 1, inner_bound.width + 2, inner_bound.height + 2};

    struct kernel
    {
        int region_count;
        bool hit_boundry;

        const std::vector<struct point>& points;
        const struct region& inner_bound;

        kernel(const std::vector<struct point>& p, const struct region& i) : points(p), inner_bound(i)
        {
            region_count = 0;
            hit_boundry = false;
        }

        kernel(kernel& o, tbb::split) : points(o.points), inner_bound(o.inner_bound)
        {
            region_count = 0;
            hit_boundry = false;
        }

        void operator()(const tbb::blocked_range2d<int, int>& range)
        {
            for (int y = range.rows().begin() ; y < range.rows().end() ; ++y)
            {
                for (int x = range.cols().begin() ; x < range.cols().end() ; ++x)
                {
                    struct point this_point(x, y);

                    //!! part 2 solver
                    // compute the sum of the distances to this point, and check it's under the threshold
                    int dist_sum = 0;
                    for (auto& p : points) dist_sum += dist(p, this_point);
                    if (dist_sum < 10000)
                    {
                        ++region_count;

                        // if we hit the boundary, then we have uncounted points in the region
                        hit_boundry |= inner_bound.excludes(this_point);
                    }
                }
            }
        }

        void join(kernel& o)
        {
            region_count += o.region_count;
            hit_boundry |= o.hit_boundry;
        }

    };

    struct kernel k(points, inner_bound);
    tbb::parallel_reduce(
            tbb::blocked_range2d<int, int>(outer_bound.y, outer_bound.y + outer_bound.height, outer_bound.x, outer_bound.x + outer_bound.width),
            k);

    assert(!k.hit_boundry);
    return k.region_count;
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(lines.size() > 0);

    auto points = convert_strings<struct point>(lines, parse_point);

    std::cout << day06_solve_part1(points) << std::endl;
    std::cout << day06_solve_part2(points) << std::endl;
    return 0;
}

