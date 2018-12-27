#include <cassert>
#include <iostream>
#include <limits>
#include <queue>
#include <unordered_set>
#include <vector>

#include "../util/file_parsing.h"
#include "../util/grid.h"



// compare points in reading order
bool operator<(const Point& a, const Point& o)
{
    if (a.y != o.y) return a.y < o.y;
    return a.x < o.x;
}

bool operator>(const Point& a, const Point& o)
{
    if (a.y != o.y) return a.y > o.y;
    return a.x > o.x;
}

bool operator==(const Point& a, const Point& o)
{
    return (a.y == o.y && a.x==o.x);
}

bool operator!=(const Point& a, const Point& o)
{
    return (a.y != o.y || a.x != o.x);
}


bool any_neighbour_matches(char c, const NoddySparseGrid<char>& g, Point p)
{
    // return true if any of the 4-neighbours of p match c
    Point up{p.x, p.y - 1};
    Point down{p.x, p.y + 1};
    Point left{p.x - 1, p.y};
    Point right{p.x + 1, p.y};

    return (up.is_positive() && g[up] == c) ||
           (down.is_positive() && g[down] == c) ||
           (left.is_positive() && g[left] == c) ||
           (right.is_positive() && g[right] == c);
}

bool are_neighbours(Point a, Point b)
{
    // a and b are 4-neighbours if they are adjacent
    Point up{a.x, a.y - 1};
    Point down{a.x, a.y + 1};
    Point left{a.x - 1, a.y};
    Point right{a.x + 1, a.y};

    return ( up == b || down == b || left == b || right == b);
}


char opposite_team(char c)
{
    assert(c=='G' || c=='E'); // should be a unit at this point

    if (c=='G') return 'E';
    if (c=='E') return 'G';

    throw std::runtime_error("Got unexpected unit character");
}


Point find_matching_point(const Point& origin, const NoddySparseGrid<char>& grid, const std::function<bool(const Point&)>& stopping_condition)
{
    // Find the nearest point to the origin that satisfies the stopping condition
    // If there are multiple points that satisfy the condition at the same distance, then
    // select the candidate based on reading order.

    // To do this, use a simple dijkstra until we hit an appropriate stopping point (e.g. we've hit
    // a neighbour of a unit of the opposing type).
    constexpr int int_max = std::numeric_limits<int>::max();
    const Point not_visited_point { int_max, int_max };

    const std::pair<int, Point> not_visited { int_max, Point(int_max, int_max)};

    NoddySparseGrid<std::pair<int, Point>> closed(not_visited);    // distance, from-point

    std::deque<std::tuple<int, Point, Point>> open;             // distance, point, from-point
    open.emplace_back(0, origin, origin);

    Point end_point = not_visited_point;       // best end-point, based on reading order

    int stop_at_distance = std::numeric_limits<int>::max();
    while(!open.empty())
    {
        // Pop this candidate and it's distance
        auto candidate_tuple = open.front();
        open.pop_front();

        const int dist = std::get<0>(candidate_tuple);
        const Point& p = std::get<1>(candidate_tuple);
        const Point& fp = std::get<2>(candidate_tuple);

        // If this distance is bigger than our stop-limit... stop!
        // because of how dijkstra works without edge-costs, distance should be monotonic
        if (dist > stop_at_distance) break;

        // If this is a possible stopping point, enqueue it in the stopping points and set the distance limit
        if (stopping_condition(p))
        {
            stop_at_distance = dist;
            end_point = std::min(end_point, p);

            // but don't stop yet - this could be the best path to a point so far
        }

        // If this candidate path is better than the existing to this point, then use this path
        auto& closed_entry = closed[p];
        if (dist < closed_entry.first)
        {
            // This candidate is better - use it as the path to this point
            closed_entry.first = dist;
            closed_entry.second = fp;

            // (Re-)enqueue the neighbours if they're walkable at this distance
            Point up{p.x, p.y - 1};
            Point down{p.x, p.y + 1};
            Point left{p.x - 1, p.y};
            Point right{p.x + 1, p.y};

            if (up.is_positive() && grid[up] == '.' && not_visited == closed[up]) open.emplace_back(dist + 1, up, p);
            if (down.is_positive() && grid[down] == '.' && not_visited == closed[down]) open.emplace_back(dist + 1, down, p);
            if (left.is_positive() && grid[left] == '.' && not_visited == closed[left]) open.emplace_back(dist + 1, left, p);
            if (right.is_positive() && grid[right] == '.' && not_visited == closed[right]) open.emplace_back(dist + 1, right, p);
        }
    }

    if (end_point != not_visited_point) return end_point;

    // if there was no path (we never stopped) then this will be a non-positive point
    return Point(-1, -1);
}

Point next_move(const Point& unit, const NoddySparseGrid<char>& grid)
{
    // To find the next move to make, find the nearest reachable neighbour of the opposing team (in reading order)
    // then find the nearest neighbour of ourselves (also in reading order).
    char unit_char = grid[unit];
    assert(unit_char == 'E' || unit_char == 'G');

    Point matching_point = find_matching_point(unit, grid, [&](const Point& p) -> bool
    {
        return any_neighbour_matches(opposite_team(unit_char), grid, p);
    });

    // if there was no reachable neighbour of an opposing unit, then don't move
    if (!matching_point.is_positive()) return unit;

    // otherwise, run the match 'backwards' to find the step direction
    Point step_direction = find_matching_point(matching_point, grid, [&](const Point& p) -> bool
    {
        return are_neighbours(unit, p);
    });

    // if we could reach a matching end point, we must be able to get a matching start point!
    assert(step_direction.is_positive());
    return step_direction;
}


void populate_grid_and_units(NoddySparseGrid<char>& g, std::vector<std::pair<Point, int>>& units, const std::vector<std::string>& lines)
{
    constexpr int initial_hp = 200;

    for (size_t y = 0; y < lines.size(); y++)
    {
        auto& l = lines[y];
        for (size_t x = 0; x < l.size(); x++)
        {
            g[y][x] = l[x];

            if (l[x] == 'E' || l[x] == 'G') units.emplace_back(Point(x,y), initial_hp);
        }
    }
}


void move_unit(const Point& from, const Point& to, NoddySparseGrid<char>& grid)
{
    assert(from != to);
    assert(are_neighbours(from, to));

    assert(grid[from] == 'E' || grid[from] == 'G');
    assert(grid[to] == '.');    // can't move to an occupied spot!

    std::swap(grid[to], grid[from]);
}


std::vector<std::pair<Point, int>>::iterator find_attacked_unit(const Point& atacking_unit_pos, const NoddySparseGrid<char>& grid, std::vector<std::pair<Point, int>>& units)
{
    // Find the position of the neighbouring unit with the lowest health
    // This is the inverse of what's in units! We want to invert the minimum ordering of the tuple, so that we pick
    // the smallest health, but prefer the reading order if multiple units have the same health
    std::pair<int, Point> best{std::numeric_limits<int>::max(), Point(-1, -1)};
    auto best_iter = units.end();

    char attacking_unit_type = grid[atacking_unit_pos];
    char attacked_unit_type = opposite_team(attacking_unit_type);
    assert(attacking_unit_type == 'E' || attacking_unit_type == 'G');

    for (auto unit_iter = units.begin(); unit_iter != units.end(); ++unit_iter)
    {
        // if this is a dead unit, skip it
        if (unit_iter->second <= 0) continue;

        if (are_neighbours(atacking_unit_pos, unit_iter->first) &&
            grid[unit_iter->first] == attacked_unit_type)
        {
            std::pair<int, Point> candidate{unit_iter->second, unit_iter->first};
            if (candidate < best)
            {
                best = candidate;
                best_iter = unit_iter;
            }
        }
    }

    // if we've not found a candidate, then the best_iter will be units.end()
    return best_iter;
}


void run_one_round(NoddySparseGrid<char>& grid, std::vector<std::pair<Point, int>>& units, int& goblin_count, int& elf_count, int elf_attack_power)
{
    constexpr int goblin_attack_power = 3;
    assert(units.size() == goblin_count + elf_count);   // should only have elves and goblins!

    // the units move/attack in their reading order at the start of the turn
    std::sort(units.begin(), units.end());

    for (auto& unit : units)
    {
        // if this unit is already dead, skip it
        if (unit.second <= 0) continue;

        // if this unit already has a neighbour which is of the opposing side, then don't move
        auto& unit_point = unit.first;

        char unit_type = grid[unit_point];
        assert(unit_type == 'G' || unit_type == 'E');

        if (!any_neighbour_matches(opposite_team(grid[unit_point]), grid, unit_point))
        {
            // no neighbour matches, so move
            Point move_to = next_move(unit_point, grid);
            if (move_to != unit_point)
            {
                move_unit(unit_point, move_to, grid);
                unit_point = move_to;
            }
        }

        // now we've moved, try and attack if we have a neighbour of the opposing team
        auto attacked_iter = find_attacked_unit(unit_point, grid, units);
        if (attacked_iter != units.end())
        {
            attacked_iter->second -= (unit_type == 'E' ? elf_attack_power : goblin_attack_power);
            if (attacked_iter->second <= 0)
            {
                if (grid[attacked_iter->first]=='E') --elf_count;
                else if (grid[attacked_iter->first]=='G') --goblin_count;
                else assert(false); // unknown unit type?

                grid[attacked_iter->first] = '.';        // remove the body
                attacked_iter->first = Point(-1, -1);    // make the dead units position invalid
            }
        }
    }

    // remove the dead units from the list
    units.erase(std::remove_if(units.begin(), units.end(), [](const std::pair<Point, int>& u)->bool { return u.second <= 0; }), units.end());
}


int run_until_winner_or_elf_death(NoddySparseGrid<char>& grid, std::vector<std::pair<Point, int>>& units, bool ignore_elf_deaths = true, int elf_attack_power = 3)
{
    // count the number of goblin/elf units
    int goblin_count = 0;
    int elf_count = 0;
    for (const auto& u : units)
    {
        if (grid[u.first] == 'E') ++elf_count;
        else if (grid[u.first] == 'G') ++goblin_count;
    }
    const int initial_elf_count = elf_count;

    // run the 'simulation' until there are no more elves xor goblins
    int round = 0;
    for (; goblin_count && elf_count; ++round)
    {
        run_one_round(grid, units, goblin_count, elf_count, elf_attack_power);

        if (!ignore_elf_deaths)
        {
            // abort early if any elves die!
            if (elf_count < initial_elf_count) return -1;
        }
    }

    return round - 1;   // the last 'round' didn't complete!
}

int day15_solve_part1(const NoddySparseGrid<char>& initial_grid, const std::vector<std::pair<Point, int>>& initial_units)
{

    auto grid = initial_grid;
    auto units = initial_units;

    // run the simulation
    int rounds = run_until_winner_or_elf_death(grid, units);

    // sum the remaining hp's of the living units
    int hp_sum = 0;
    for (auto& u : units) hp_sum += u.second;

    return hp_sum * rounds;
}



int day15_solve_part2(const NoddySparseGrid<char>& initial_grid, const std::vector<std::pair<Point, int>>& initial_units)
{
    auto units = initial_units;

    // Just search linearly - given attack_power is << 200, it shouldn't take long!
    int attack_power = 3;
    int rounds = -1;
    while (rounds < 0)
    {
        units = initial_units;
        auto grid = initial_grid;
        rounds = run_until_winner_or_elf_death(grid, units, false, ++attack_power);
    }

    // sum the remaining hp's of the living units
    int hp_sum = 0;
    for (auto& u : units) hp_sum += u.second;

    return hp_sum * rounds;
}


int main()
{

    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    NoddySparseGrid<char> grid;
    std::vector<std::pair<Point, int>> units;
    populate_grid_and_units(grid, units, lines);

    std::cout << day15_solve_part1(grid, units) << std::endl;
    std::cout << day15_solve_part2(grid, units) << std::endl;
    return 0;
}

