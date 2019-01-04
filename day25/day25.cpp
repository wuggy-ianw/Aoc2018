#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "../util/file_parsing.h"

struct Point4d
{
    std::array<int, 4> v;

    bool operator==(const Point4d& o) const { return v == o.v; }
};

namespace std
{
    template<> struct hash<Point4d>
    {
        size_t operator()(const Point4d& p) const noexcept
        {
            constexpr size_t m = 24251;
            size_t hash = 0;
            for (auto v : p.v) hash = (hash*m) + std::hash<decltype(v)>()(v);
            return hash;
        }
    };
}

constexpr int constellation_range = 3;

int dist(const Point4d& a, const Point4d& b)
{
    int dist = 0;
    for (size_t i = 0; i<a.v.size(); ++i) dist += std::abs(a.v[i] - b.v[i]);
    return dist;
}


Point4d parse_p4d(const std::string& s)
{
    Point4d p{};
    std::stringstream ss(s);
    ss >> p.v[0] >> ',' >> p.v[1] >> ',' >> p.v[2] >> ',' >> p.v[3];
    assert(ss);
    return p;
}

bool is_point_in_constellation(const Point4d& point, const std::unordered_set<Point4d>& constellation)
{
    for (const auto& cp : constellation)
    {
        if (dist(point, cp) <= constellation_range) return true;
    }
    return false;
}

size_t day25_solve_part1(const std::vector<Point4d>& points)
{
    std::vector<std::unordered_set<Point4d>> constellations;

    for(const auto& p : points)
    {
        // check if this point is in one (or more) existing constellations
        // if it's in one constellation, insert it
        // if it's in more than one, merge them together than insert it
        auto first_found_constellation = constellations.end();
        for (auto citer = constellations.begin(); citer != constellations.end(); ++citer)
        {
            auto inside = is_point_in_constellation(p, *citer);
            if (!inside) continue;

            if (first_found_constellation == constellations.end())
            {
                // this is the first constellation this point is in
                first_found_constellation = citer;
                first_found_constellation->insert(p);
            }
            else
            {
                // this constellation is 'joined' to the first found constellation by the inserted point
                // merge them into the first found, and empty this one
                first_found_constellation->merge(*citer);
                assert(citer->empty()); // all items should have been moved
            }
        }

        // if this point wasn't in any constellation, make a new one
        if (first_found_constellation == constellations.end())
        {
            constellations.emplace_back(std::unordered_set<Point4d>{p});
        }

        // erase the empty constellations
        constellations.erase( std::remove_if(constellations.begin(), constellations.end(), [](const auto& p){return p.empty(); }), constellations.end());
    }

    return constellations.size();
}



int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    auto points = convert_strings<Point4d>(lines, parse_p4d);
    assert(!points.empty());

    std::cout << day25_solve_part1(points) << std::endl;
    return 0;
}

