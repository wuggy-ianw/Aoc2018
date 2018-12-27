#include <cassert>
#include <fstream>
#include <iostream>
#include <deque>
#include <unordered_set>

#include "../util/file_parsing.h"
#include "../util/grid.h"


enum direction
{
    dirNorth = 0,
    dirSouth = 1,
    dirEast = 2,
    dirWest = 3,

    dirMax = 4
};


bool is_direction_char(char c)
{
    return c == 'N' || c == 'E' || c == 'W' || c == 'S';
}

direction char_to_direction(char c)
{
    if (c == 'N') return dirNorth;
    if (c == 'S') return dirSouth;
    if (c == 'E') return dirEast;
    if (c == 'W') return dirWest;

    assert(false);
    throw std::runtime_error("Unknown direction character");
}

direction opposite_direction(direction d)
{
    assert(d >= 0);
    assert(d < 4);

    return static_cast<direction>(d ^ 1);   // just invert the LSB
}

void move_by_direction(direction d, int& x, int& y)
{
    switch(d)
    {
        default:
            assert(false);
            throw std::runtime_error("invalid direction index");
        case dirNorth: --y; break;
        case dirSouth: ++y; break;
        case dirEast: --x; break;
        case dirWest: ++x; break;
    }
}

size_t scan_subexpression(const std::string& tape, size_t start_pos)
{
    // memoise - assumes we always have the same tape!
    static std::unordered_map<size_t, size_t> memos;

    auto iter = memos.find(start_pos);
    if (iter != memos.end()) return iter->second;

    size_t pos = start_pos;
    while(true)
    {
        if (is_direction_char(tape[pos])) ++pos;
        else if (tape[pos] == '(')
        {
            // found a parenthesisied alternatives expression...
            ++pos;

            while (true)
            {
                pos = scan_subexpression(tape, pos);
                if (tape[pos] == '|') { ++pos; continue; }
                if (tape[pos] == ')') { ++pos; break; }

            }
        }
        else break;     // hit the end of the expression (e.g. an unmatched ')' or '|' or something unexpected
    }

    memos[start_pos] = pos;
    return pos;
}


struct Walker
{
    size_t pos = 0;
    int x = 0;
    int y = 0;

    bool operator==(const Walker& o) const
    {
        return pos == o.pos && x == o.x && y == o.y;
    };
};

void walk_next(const std::string& tape, Walker walker, const std::function<void(const Walker& from, const Walker& to, direction d)>& did_walk, const std::function<void(const Walker& new_walker)>& add_walker)
{
    // skip the initial token
    if (walker.pos == 0 && tape[walker.pos] == '^') walker.pos++;

    // if this is the end token, then this walker is done, do nothing
    if (tape[walker.pos] == '$') return;

    // walk any simple sequence
    if (is_direction_char(tape[walker.pos]))
    {
        // walk the whole sequence
        while(is_direction_char(tape[walker.pos]))
        {
            direction d = char_to_direction(tape[walker.pos]);

            Walker dest = walker;
            move_by_direction(d, dest.x, dest.y);
            ++dest.pos;

            did_walk(walker, dest, d);

            walker = dest;
        }

        // hit some other character... deal with that next phase
        add_walker(walker);
        return;
    }

    // If we didn't hit the end, and we're not a simple sequence, then we *SHOULD* be an alternative
    // the format is parenthesis bound, with one or more '|' delimited sequences (which may include nested
    // alternative-expressions consisting of parenthesis and pipe characters).
    if (tape[walker.pos] == '(')
    {
        // We're opening a new alternative
        // Spawn a new walk for each alternative sequence
        size_t scanpos = walker.pos + 1;    // skip the '('
        while(true)
        {
            // add a new alternative walker
            add_walker({scanpos, walker.x, walker.y});

            // scan for the end of this expression
            scanpos = scan_subexpression(tape, scanpos);

            // two cases '|' or ')'
            if (tape[scanpos] == '|') { ++scanpos; continue; }  // skip after the '|', start a walker after
            if (tape[scanpos] == ')') { ++scanpos; break; }     // end of the alternatives expression
        }

        // done - we've spawned a walker for each alternative
    }
    else if (tape[walker.pos] == '|' or tape[walker.pos] == ')')
    {
        // we were in an alternative expression, and we've completed it
        // keep skipping the other alternatives until we hit the close

        size_t scanpos = walker.pos;
        while(tape[scanpos] == '|') scanpos = scan_subexpression(tape, scanpos + 1);

        assert(tape[scanpos] == ')');    // should always hit the end of the alternative expression!
        ++scanpos;

        // resume walking at the next expression after the alt expression
        add_walker({scanpos, walker.x, walker.y});
    }
    else
    {
        assert(false);
        throw std::runtime_error("Unexpected character in alternative expression?");
    }
}


namespace std
{
    template<> struct hash<Walker>
    {
        size_t operator()(const Walker& p) const noexcept
        {
            constexpr size_t m = 24251;     // a prime...
            return std::hash<size_t>()(p.pos) + (std::hash<int>()(p.x) * m) + (std::hash<int>()(p.y) * m * m);
        }
    };
}


using Grid = OriginCenteredGrid<std::array<bool, dirMax>>;
using DistGrid = OriginCenteredGrid<size_t>;


void walk_paths_to_make_map(const std::string& tape, Grid& grid)
{
    // Walk the instructions on the tape, branching as needed
    // Branches *can* converge and overlap...
    // Keep a set of closed walkers so that convergences can be eliminated
    std::unordered_set<Walker> closed;
    std::unordered_set<Walker> open;
    open.insert(Walker{});

    while(!open.empty())
    {
        std::unordered_set<Walker> newly_open;
        for (const Walker& walker : open) {
            closed.insert(walker);
            walk_next(tape, walker,
                      [&](const Walker &from, const Walker &to, direction d) {
                          // add a door to the grid, for both directions
                          grid[from.y][from.x][d] = true;
                          grid[to.y][to.x][opposite_direction(d)] = true;
                      },
                      [&](const Walker &new_walker) {
                          // add a new walker if this isn't already a tape-position and grid-position that's occured before
                          if (closed.end() == closed.find(new_walker)) {
                              newly_open.insert(new_walker);
                          }
                      }
            );
        }

        open = std::move(newly_open);
    }
}


void process_room_distances(const Grid& grid, DistGrid& dist_grid)
{
    std::deque<std::pair<size_t, Point>> open;
    open.push_back({0, Point{0,0}});

    Point furthest;

    while (!open.empty())
    {
        auto candidate = open.front();
        open.pop_front();


        const size_t dist = candidate.first;
        Point pos = candidate.second;

        if (dist < dist_grid[pos])
        {
            // best (shortest) alternative so far
            dist_grid[pos] = dist;

            // add directions we can walk to the open set for next iteration
            for (int di = 0; di < dirMax; ++di)
            {
                if (grid[pos][di])
                {
                    Point np = pos;
                    move_by_direction(static_cast<direction>(di), np.x, np.y);

                    if (dist + 1 < dist_grid[np]) open.emplace_back(dist + 1, np);
                }
            }

        }
    }
}

std::pair<size_t, size_t> day20_solve_part1_and_2(const std::string& tape)
{
    // Walk the instructions on the tape, branching as needed
    // Branches *can* converge and overlap...
    // Keep a set of closed walkers so that convergences can be eliminated
    Grid grid;
    walk_paths_to_make_map(tape, grid);

    // now just do a simple dijkstra-like flood to find the room furthest away
    constexpr size_t not_visited = std::numeric_limits<size_t>::max();

    DistGrid dist_grid(not_visited);
    process_room_distances(grid, dist_grid);

    // scan for the (visited) room with the furthest distance
    // also count the number of rooms at least 1000 distance away
    constexpr size_t far_distance = 1000;

    Point furthest;
    size_t furthest_dist = 0;
    size_t far_room_count = 0;
    for (auto y = dist_grid.min_row(); y <= dist_grid.max_row(); ++y)
    {
        for (auto x = dist_grid.min_column(); x <= dist_grid.max_column(); ++x)
        {
            if (dist_grid[y][x] != not_visited)
            {
                if (furthest_dist < dist_grid[y][x]) {
                    furthest_dist = dist_grid[y][x];
                    furthest = Point(x, y);
                }

                if (dist_grid[y][x] >= far_distance) ++far_room_count;
            }
        }
    }

    return {furthest_dist, far_room_count};
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    const std::string& tape = lines[0];
    auto result = day20_solve_part1_and_2(tape);

    std::cout << result.first << std::endl;
    std::cout << result.second << std::endl;
    return 0;
}

