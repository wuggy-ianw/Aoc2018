#include <cassert>
#include <iostream>
#include <functional>
#include <list>
#include <algorithm>

#include "../util/file_parsing.h"

typedef long marble_value;

class MarbleCircle
{
private:
    using List = std::list<marble_value>;

    List circle;
    List::iterator current_iter;

    void move_cw(int count)
    {
        // cw is ++ on current_iter

        for(;count>0; --count)
        {
            current_iter++;
            if (current_iter == circle.end())
            {
                current_iter = circle.begin();
            }
        }
    }

    void move_ccw(int count)
    {
        // ccw is -- on current_iter

        for (;count>0; --count)
        {
            if (current_iter == circle.begin())
            {
                current_iter = circle.end();
            }
            current_iter--;
        }
    }

    void insert_after(marble_value marble)
    {
        move_cw(1);
        current_iter = insert_before(marble);
    }

    List::iterator insert_before(marble_value marble)
    {
        // insert before the current, leave current alone
        // returns the iterator to the inserted position
        return circle.insert(current_iter, marble);
    }

public:
    MarbleCircle()
    {
        circle.push_back(0);
        current_iter = circle.begin();
    }

    void insert_at_cw1(marble_value marble)
    {
        // move clockwise one marble, then insert
        move_cw(1);
        insert_after(marble);
    }

    marble_value remove_from_ccw7()
    {
        // mov ccw 7 marbles, then remove it
        // the marble cw from the removed marble is the new 'current'
        move_ccw(7);
        marble_value marble = *current_iter;

        current_iter = circle.erase(current_iter);  // iter is the item after (cw) in the list, or end
        if (current_iter == circle.end())
        {
            // if end, then the beginning of the list is cw of the removed item
            current_iter = circle.begin();
        }

        return marble;
    }
};



marble_value day08_solve_part1(size_t n_players, marble_value last_marble_score)
{
    MarbleCircle circle;

    std::vector<marble_value> scores;
    scores.resize(n_players, 0);

    marble_value last_placed = 0;
    while(last_placed < last_marble_score)
    {
        for (int p = 0; p<n_players && last_placed < last_marble_score; p++)
        {
            ++last_placed;
            if (last_placed % 23 != 0)
            {
                circle.insert_at_cw1(last_placed);
            }
            else
            {
                scores[p] += last_placed;
                scores[p] += circle.remove_from_ccw7();
            }

        }
    }
    assert(last_placed == last_marble_score); // should be exact!

    marble_value highest_score = *std::max_element(scores.begin(), scores.end());
    return highest_score;
}


void test()
{
    assert(day08_solve_part1(10, 1618) == 8317);
    assert(day08_solve_part1(13, 7999) == 146373);
    assert(day08_solve_part1(17, 1104) == 2764);
    assert(day08_solve_part1(21, 6111) == 54718);
    assert(day08_solve_part1(30, 5807) == 37305);
}

int main()
{
    test();

    std::cout << day08_solve_part1(493, 71863) << std::endl;
    std::cout << day08_solve_part1(493, 7186300) << std::endl;
    return 0;
}

