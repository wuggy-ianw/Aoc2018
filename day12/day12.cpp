#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "../util/file_parsing.h"

constexpr size_t expected_rule_len = 5;

using RuleVal = int;
using RuleSet = std::unordered_map<RuleVal, char>;


RuleVal rule_from_string(const std::string& s)
{
    assert(s.size() == expected_rule_len);

    RuleVal v = 0;
    for (int i = 0; i < expected_rule_len; ++i)
    {
        if (s[i] == '#') v |= 1 << i;
    }

    return v;
}

class PotSet
{
public:
    typedef std::unordered_map<int, char> container_type;

private:
    container_type potset;

    int minval = std::numeric_limits<int>::max();
    int maxval = std::numeric_limits<int>::min();

public:
    PotSet() = default;

    char& operator[](int i)
    {
        minval = std::min(i, minval);
        maxval = std::max(i, maxval);

        return potset[i];
    }

    container_type::iterator find(int i) { return potset.find(i); }
    container_type::const_iterator find(int i) const { return potset.find(i); }

    container_type::iterator begin() { return potset.begin(); }
    container_type::const_iterator begin() const { return potset.begin(); }

    container_type::iterator end() { return potset.end(); }
    container_type::const_iterator end() const { return potset.end(); }

    int min() const
    {
        return minval;
    }

    int max() const
    {
        return maxval;
    }

    bool operator==(const PotSet& o) const
    {
        return minval == o.minval && maxval == o.maxval && potset == o.potset;
    }

    // convert the pot state to a pile of bits, ignoring the shift for the zero position
    std::vector<bool> to_bits() const
    {
        std::vector<bool> v;
        for (const auto& kv : potset)
        {
            auto i = static_cast<size_t>(kv.first - minval);

            if (v.size() < i) v.resize(i, false);
            v[i] = true;
        }

        return v;
    }
};


PotSet parse_initial_state(const std::string& s)
{
    std::stringstream ss(s);

    std::string state;
    ss >> "initial" >> "state:" >> state;

    assert(ss);

    PotSet p;
    for (int i=0; i<state.size(); ++i)
    {
        assert(state[i] == '.' || state[i] == '#');
        if (state[i] == '#')
        {
            p[i] = '#';
        }
    }

    return p;
}


std::pair<RuleVal, char> parse_rule(const std::string& s)
{
    std::stringstream ss(s);

    std::string srule;
    char result;

    ss >> srule >> "=>" >> result;
    assert(ss);
    assert(srule.size() == expected_rule_len);
    assert(result == '.' || result == '#');

    RuleVal r = rule_from_string(srule);
    return {r, result};
}


// extact the pots for this position
RuleVal extract_pots(const PotSet& p, int i)
{
    // i is the CENTER index, not the leftmost!
    RuleVal r = 0;
    int offset = i-2;

    for (int j = 0; j < expected_rule_len ; ++j)
    {
        auto iter = p.find(offset + j);
        if (iter != p.end() && iter->second == '#')
        {
            r |= 1 << j;
        }
    }

    return r;
}


// extract the pots for this position, using the previous position's
RuleVal extract_pots_next(const PotSet& p, RuleVal v, int i)
{
    v = v >> 1;
    auto iter = p.find(i + 2);
    if (iter != p.end() && iter->second == '#')
    {
        v |= 1 << (expected_rule_len - 1);
    }

    return v;
}


PotSet mutate_state(const RuleSet& rules, const PotSet& pots)
{
    PotSet new_pots;

    // pad by the rule length so we can produce plants outside of the bounds
    int first = pots.min() - static_cast<int>(expected_rule_len);
    int last = pots.max() + static_cast<int>(expected_rule_len);

    RuleVal r = 0;
    for (int i = first; i <= last; ++i)
    {
        r = (i==first) ? extract_pots(pots, i) : extract_pots_next(pots, r, i);

        auto match_iter = rules.find(r);
        assert(match_iter != rules.end());  // every substring should have a matching rule!

        if (match_iter != rules.end() && match_iter->second == '#') new_pots[i] = '#';   // only insert occupied pots
    }

    return new_pots;
}


size_t sum_of_pots(const PotSet& pots)
{
    size_t sum = 0;
    for (const auto& kv : pots)
    {
        assert(kv.second == '#');   // only occupied pots should be present
        sum += kv.first;
    }

    return sum;
}


size_t day12_solve_part1(const RuleSet& rules, const PotSet& initial_state)
{
    PotSet state = initial_state;

    for (size_t i=1; i<=20; ++i) state = mutate_state(rules, state);
    return sum_of_pots(state);
}


size_t day12_solve_part2(const RuleSet& rules, const PotSet& initial_state)
{
    constexpr size_t target_gen = 50000000000ull;
    constexpr size_t cycle_check_limit = 10000;
    PotSet state = initial_state;

    for (size_t i=1; i<=target_gen; ++i)
    {
        assert(i < cycle_check_limit);      // didn't find a cycle?!
        auto new_state = mutate_state(rules, state);

        // check if this state is just the previous state shifted!
        if (new_state.to_bits() == state.to_bits())
        {
            // state -> new_state is a stable shift...
            // Instead of iterating through the generations we can just apply the difference in score
            // That should simply be the number of plants in the pots per shift right

            size_t state_score = sum_of_pots(state);
            size_t newstate_score = sum_of_pots(new_state);

            size_t delta_score =  newstate_score - state_score;

            size_t remaining_generations = target_gen - i;
            size_t total_score = newstate_score + (delta_score * remaining_generations);

            return total_score;
        }

        state = new_state;
    }

    assert(false);  // unreachable
}



int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    PotSet initial_state = parse_initial_state(lines[0]);

    RuleSet rules;
    for(auto i = lines.begin() + 2; i != lines.end(); ++i)
    {
        rules.insert(parse_rule(*i));
    }

    std::cout << day12_solve_part1(rules, initial_state) << std::endl;
    std::cout << day12_solve_part2(rules, initial_state) << std::endl;
    return 0;
}

