#include <cassert>
#include <iostream>
#include <queue>
#include <unordered_set>

#include "../util/file_parsing.h"
#include "../util/grid.h"

constexpr int depth = 3066;
constexpr int targetx = 13;
constexpr int targety = 726;



enum CellType
{
    cellRocky = 0,
    cellWet = 1,
    cellNarrow = 2
};

int risk_for_cell(CellType ct)
{
    return static_cast<int>(ct);
}

char cell_to_char(CellType ct)
{
    const char * c = ".=|";
    return c[static_cast<int>(ct)];
}



CellType cell_for_erosion_level(int el)
{
    return static_cast<CellType>(el % 3);
}


int geologic_index(int x, int y);


int erosion_level(int x, int y)
{
    return (geologic_index(x,y) + depth) % 20183;
}


int geologic_index_calc(int x, int y)
{

    if (x==0 && y==0) return 0;
    if (x==targetx && y==targety) return 0;

    if (y==0) return x*16807;
    if (x==0) return y*48271;

    return erosion_level(x-1, y) * erosion_level(x, y-1);
}


int geologic_index(int x, int y)
{
    constexpr int not_set = -1;
    static NoddySparseGrid<int> memos(not_set);

    int& gi = memos[y][x];
    if (gi == not_set)
    {
        gi = geologic_index_calc(x, y);
    }

    return gi;
}


int day22_solve_part1()
{
    int risk = 0;
    for (int y=0; y<=targety; ++y)
    {
        for (int x=0; x<=targetx; ++x)
        {
            auto el = erosion_level(x, y);
            auto ct = cell_for_erosion_level(el);

            risk += risk_for_cell(ct);
        }
    }

    return risk;
}

enum Equipped
{
    equippedNone = 0,
    equippedTorch = 1,
    equippedClimbing = 2
};

bool operator==(const Point& a, const Point& b)
{
    return a.x == b.x && a.y == b.y;
}

struct State
{
    Point position;
    Equipped equipped = equippedTorch;    // initial state is with the torch equipped

    bool operator==(const State& o) const
    {
        return position == o.position && equipped == o.equipped;
    }
};

struct TimedState
{
    State state;
    int time = 0;
};

struct QueuedState
{
    TimedState timedState;
    int metric = 0;

    bool operator<(const QueuedState& other) const
    {
        // use >= so that 'lower' valued metrics are considered 'better' by the priq.
        return metric >= other.metric;
    }
};

namespace std
{
    template<> struct hash<State>
    {
        typedef Point argument_type;
        size_t operator()(const State& p) const noexcept
        {
            constexpr size_t m = 24251;
            return (std::hash<Point>()(p.position) * m) + std::hash<int>()(static_cast<int>(p.equipped));
        }
    };
}


void try_enqueue(std::priority_queue<QueuedState>& queue, std::unordered_set<State>& closed, const TimedState& candidate)
{
    // only enqueue valid, non-closed states
    const Point& p = candidate.state.position;
    if (!p.is_positive()) return;  // was inaccessible rock
    if (closed.end() != closed.find(candidate.state)) return;   // was closed

    // can only be using certain tools in certain types of cell
    CellType ct = cell_for_erosion_level( erosion_level(p.x, p.y));
    switch(ct)
    {
        default:
            assert(false);
        case cellRocky:
            if (candidate.state.equipped == equippedNone) return;       // can't be using no equipment on Rocky
            break;
        case cellWet:
            if (candidate.state.equipped == equippedTorch) return;      // can't be using torch on Wet
            break;
        case cellNarrow:
            if (candidate.state.equipped == equippedClimbing) return;   // can't be using climbing stuff on Narrow
            break;
    }

    // if we've got this far, then we're a valid, open state... enqueue!

    // use a simple manhatten metric to estimate how much further to go for this candidate
    int dist = std::abs(p.x - targetx) + std::abs(p.y - targety);
    int metric = dist + candidate.time;

    QueuedState qs{candidate, metric};
    queue.push(qs);
}

void enqueue_moves(std::priority_queue<QueuedState>& queue, std::unordered_set<State>& closed, const TimedState& source)
{
    // We can go to any +ive 4-connected ordinate, extra cost of 1 minute
    const Point& p = source.state.position;

    TimedState candidate = source;
    ++candidate.time;

    candidate.state.position = Point(p.x - 1, p.y); try_enqueue(queue, closed, candidate);
    candidate.state.position = Point(p.x + 1, p.y); try_enqueue(queue, closed, candidate);
    candidate.state.position = Point(p.x, p.y - 1); try_enqueue(queue, closed, candidate);
    candidate.state.position = Point(p.x, p.y + 1); try_enqueue(queue, closed, candidate);

    // We can swap out our equipment, extra cost of 7 minutes
    candidate = source;
    candidate.time += 7;
    candidate.state.equipped = equippedNone; try_enqueue(queue, closed, candidate);
    candidate.state.equipped = equippedTorch; try_enqueue(queue, closed, candidate);
    candidate.state.equipped = equippedClimbing; try_enqueue(queue, closed, candidate);
}

int day22_solve_part2()
{
    std::unordered_set<State> closed;
    std::priority_queue<QueuedState> queue;
    queue.push(QueuedState());   // start at 0,0 with the torch out

    State final_state{Point(targetx, targety), equippedTorch};

    // while there are possible locations to search
    while(!queue.empty())
    {
        // get the lowest metric candidate
        const QueuedState qs = queue.top(); queue.pop();
        const TimedState& candidate = qs.timedState;

        // add the candidate to the closed set
        auto insert_result = closed.insert(candidate.state);
        if (!insert_result.second) continue;    // if this is an already closed state, then skip it

        // if this is our final state, then we're done!
        if (candidate.state == final_state) return candidate.time;

        // otherwise, enqueue all non-closed permutations of our state
        enqueue_moves(queue, closed, candidate);
    }

    assert(false);  // couldn't find a path at all?
    return -1;
}


int main()
{
    std::cout << day22_solve_part1() << std::endl;
    std::cout << day22_solve_part2() << std::endl;
    return 0;
}

