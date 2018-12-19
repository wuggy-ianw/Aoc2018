#include <cassert>
#include <iostream>

#include "../util/file_parsing.h"

// directions... turning cw is an increment mod 4, turning ccw is a decrement mod 4
enum direction
{
    dirUp = 0,
    dirRight = 1,
    dirDown = 2,
    dirLeft = 3,

    dirMin = 0,
    dirMax = 3
};

const int dir_dx[4] = {0, 1, 0, -1};
const int dir_dy[4] = {-1, 0, 1, 0};

enum turn_choice
{
    turnLeft = -1,
    turnNone = 0,
    turnRight = 1,

    turnMin = -1,
    turnMax = 1
};


turn_choice next_turn(turn_choice c)
{
    int i = c;
    ++i;
    if (i > turnMax) i = turnMin;
    else if (i < turnMin) i = turnMax;
    return static_cast<turn_choice>(i);
}


direction apply_turn(direction d, turn_choice c)
{
    int i = d + c;
    if (i < dirMin) i = dirMax;
    else if (i > dirMax) i = dirMin;
    return static_cast<direction>(i);

}

struct cart
{
    int x;
    int y;
    direction d;    // post-direction... e.g. entering an intersection, this is the direction we will go to LEAVE it
    turn_choice t;  // pre-direction... e.g. reaching an intersection will use turn_choice, then increment it for next time

    bool collided;  // whether this cart is 'moving' or stopped due to collision

    cart(int ax, int ay, direction ad, turn_choice at = turnLeft) : x(ax), y(ay), d(ad), t(at), collided(false)
    {
    }

    ~cart() = default;

    cart(const struct cart&) = default;
    cart(struct cart&&) = default;

    struct cart& operator=(const struct cart&) = default;
    struct cart& operator=(struct cart&&) = default;

    bool operator<(const struct cart& o)
    {
        if ( y < o.y) return true;
        if ( y > o.y) return false;
        return x < o.x;
    }
};


void process_tracks(const std::vector<std::string>& lines,
                    std::vector<struct cart>& carts,
                    std::vector<std::vector<char>>& tracks)
{
    const size_t width = std::max_element(lines.begin(), lines.end(), [](const std::string& a, const std::string& b) -> bool { return a.size() < b.size(); })->size();

    carts.clear();
    tracks.clear();
    for(size_t y = 0; y < lines.size(); ++y)
    {
        const std::string& l = lines[y];

        std::vector<char> t(width, ' ');
        for (size_t x = 0; x < l.size(); x++)
        {
            char c = l[x];

            // this might be a cart, if so, add it to the cart and convert the char to a valid track character
            switch(c)
            {
                default:
                    // do nothing
                    break;
                case '>':
                    carts.emplace_back(x, y, dirRight);
                    c = '-';
                    break;
                case '<':
                    carts.emplace_back(x, y, dirLeft);
                    c = '-';
                    break;
                case '^':
                    carts.emplace_back(x, y, dirUp);
                    c = '|';
                    break;
                case 'v':
                    carts.emplace_back(x, y, dirDown);
                    c = '|';
                    break;
            }

            // now handle the track cases
            // we may have modified c if there was a cart on this square
            switch(c)
            {
                default:
                    assert(false);  // unknown character encountered
                    throw std::runtime_error("Encountered unknown character in input");
                    break;
                case ' ':
                    // empty, just use this character
                case '-':
                case '|':
                case '+':
                case '/':
                case '\\':
                    // regular track, just use this character
                    t[x] = c;
                    break;
            }
        }

        tracks.emplace_back(std::move(t));
    }
}


bool run_step(const std::vector<std::vector<char>>& track, std::vector<struct cart>& carts)
{
    bool did_collide = false;

    // sort the carts by their position
    std::sort(carts.begin(), carts.end());

    // in order, move the cart
    for(size_t i = 0; i < carts.size(); i++)
    {
        auto& cart = carts[i];

        // skip if this cart is already collided
        if (cart.collided) continue;

        // move this cart to it's new position
        cart.x += dir_dx[cart.d];
        cart.y += dir_dy[cart.d];

        // change the direction if we've hit a corner or intersection
        char c = track[cart.y][cart.x];
        switch(c)
        {
            default:
                assert(false);  // unknown character?
                throw std::runtime_error("Encountered unknown character in input");
                break;
            case '/':
                if (cart.d == dirDown) cart.d = dirLeft;
                else if (cart.d == dirUp) cart.d = dirRight;
                else if (cart.d == dirRight) cart.d = dirUp;
                else if (cart.d == dirLeft) cart.d = dirDown;
                else { assert(false); throw std::runtime_error("Impossible corner?"); };
                break;
            case '\\':
                if (cart.d == dirDown) cart.d = dirRight;
                else if (cart.d == dirUp) cart.d = dirLeft;
                else if (cart.d == dirRight) cart.d = dirDown;
                else if (cart.d == dirLeft) cart.d = dirUp;
                else { assert(false); throw std::runtime_error("Impossible corner?"); };
                break;
            case '+':
                // intersection!
                // turn based on our state
                cart.d = apply_turn(cart.d, cart.t);
                cart.t = next_turn(cart.t);
                break;
            case '-':
            case '|':
                // no change of direction
                break;
        }

        // now check for collisions with all other carts
        for (size_t j = 0; j < carts.size(); j++)
        {
            if (i==j) continue; // ignore ourselves!
            if (cart.x == carts[j].x && cart.y == carts[j].y)
            {
                did_collide = true;
                cart.collided = true;
                carts[j].collided = true;
            }
        }
    }

    return did_collide;
}


void print_state(const std::vector<struct cart>& carts, const std::vector<std::vector<char>>& track)
{
    for(size_t y = 0; y < track.size(); ++y)
    {
        const auto& line = track[y];
        for (size_t x = 0; x < line.size(); ++x)
        {
            // default char to print
            char c = line[x];

            // but override it with a cart if there is one at this position
            for (const auto& cart : carts)
            {
                if (cart.x == x && cart.y == y)
                {
                    if (cart.collided) c = 'X';
                    else if (cart.d == dirUp) c = '^';
                    else if (cart.d == dirDown) c ='v';
                    else if (cart.d == dirLeft) c = '<';
                    else if (cart.d == dirRight) c = '>';
                }
            }

            std::cout << c;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


std::pair<int, int> day13_solve_part1(const std::vector<struct cart>& initial_carts, const std::vector<std::vector<char>>& track)
{
    // run the simulation until the first collision
    bool did_collide = false;
    std::vector<struct cart> carts = initial_carts;

    while (!did_collide)
    {
        did_collide = run_step(track, carts);
    }

    // find the first collided cart
    for (auto& cart : carts)
    {
        if (!cart.collided) continue;   // skip non-collided carts
        return {cart.x, cart.y};
    }

    assert(false);
    throw std::runtime_error("No collided carts found even though did_collide from run_step");

    return {-1, -1};
}

std::pair<int, int> day13_solve_part2(const std::vector<struct cart>& initial_carts, const std::vector<std::vector<char>>& track)
{
    // run the simulation, removing collided carts until there is only 1 cart
    std::vector<struct cart> carts = initial_carts;
    while (carts.size() > 1)
    {
        bool did_collide = run_step(track, carts);
        if (did_collide)
        {
            // remove the collided carts
            carts.erase(
                    std::remove_if(carts.begin(), carts.end(), [](const struct cart& c)->bool{return c.collided;}),
                    carts.end());
        }
    }

    // should only be one cart left
    assert(carts.size() == 1);
    return {carts[0].x, carts[0].y};
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    std::vector<struct cart> carts;
    std::vector<std::vector<char>> tracks;
    process_tracks(lines, carts, tracks);

    auto result_p1 = day13_solve_part1(carts, tracks);
    std::cout << result_p1.first << ',' << result_p1.second << std::endl;

    auto result_p2 = day13_solve_part2(carts, tracks);
    std::cout << result_p2.first << ',' << result_p2.second << std::endl;

    return 0;
}

