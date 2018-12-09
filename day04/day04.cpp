#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <regex>
#include <unordered_map>

#include "../util/file_parsing.h"

enum guard_state
{
    noGuard,
    guardShiftStart,
    guardWakesUp,
    guardFallsAsleep
};

struct event
{
    long year;
    long month;
    long day;

    long hour;
    long minute;

    long guard;
    guard_state state;

    long timeval() const
    {
        return minute + (hour<<8) + (day<<16) + (month<<24) + (year<<32);
    }
};

struct event_line_parser
{
    std::regex event_line_regex{"^[[](\\d{4})-(\\d{2})-(\\d{2}) (\\d{2}):(\\d{2})](.*)$"};
    std::regex event_guard_id_regex{"^ Guard #(\\d+) begins shift"};

    struct event operator() (const std::string& line)
    {
        struct event e = {0};
        std::smatch matches;
        bool did_match = std::regex_match(line, matches, event_line_regex);
        assert(did_match);  // all lines should match!

        e.year = std::stoi(matches[1]);
        e.month = std::stoi(matches[2]);
        e.day = std::stoi(matches[3]);

        e.hour = std::stoi(matches[4]);
        e.minute = std::stoi(matches[5]);

        if (matches[6] == " falls asleep") e.state = guardFallsAsleep;
        else if (matches[6] == " wakes up") e.state = guardWakesUp;
        else
        {
            std::string remainder = matches[6];

            std::smatch guard_match;
            did_match = std::regex_match(remainder, guard_match, event_guard_id_regex);
            assert(did_match);  // only thing that should match, if it wasn't a wake or sleep event

            e.state = guardShiftStart;
            e.guard = std::stoi(guard_match[1]);
        }

        return e;
    }
};

struct time_compare_events
{
    bool operator() (const struct event& a, const struct event& b)
    {
        return a.timeval() < b.timeval();
    }
};


std::pair<long, long> day04_solve_part1_and_2(const std::vector<event>& events)
{
    // process the events so we know how long each guard spent asleep
    struct guard_sleep_data
    {
        int sleep_count = 0;
        std::array<int, 60> sleep_minutes = {};
    };

    std::array<std::unordered_map<long, int>, 60> minute_sleepy_guards;
    std::unordered_map<long, struct guard_sleep_data> guard_sleepy_times;
    struct guard_sleep_data* current_guard_data = nullptr;
    long current_sleep_start = 0;
    guard_state current_state = noGuard;
    long current_guard_id = 0;

    for (auto& e : events)
    {
        switch(e.state)
        {
            case guardShiftStart:
                assert(current_state != guardFallsAsleep);  // can't replace a sleeping guard?

                // get the data for this new guard
                current_guard_id = e.guard;
                current_guard_data = &guard_sleepy_times[e.guard];
                current_state = guardWakesUp;   // guard is awake

                break;

            case guardFallsAsleep:
                assert(current_state != noGuard);
                assert(current_state != guardFallsAsleep);  // not already asleep
                assert(e.hour == 0);    // not handling sleeps starting outside the midnight hour?
                current_sleep_start = e.minute;

                current_state = guardFallsAsleep;
                break;

            case guardWakesUp:
                assert(current_state != noGuard);
                assert(current_state != guardWakesUp);  // not already awake
                assert(e.hour == 0);    // not handling sleeps ending outside the midnight hour?

                for(long i = current_sleep_start; i < e.minute; i++)
                {
                    ++current_guard_data->sleep_minutes[i];

                    // also collect the guard asleep in this minute
                    ++minute_sleepy_guards[i][current_guard_id];
                }
                current_guard_data->sleep_count += e.minute - current_sleep_start;

                current_state = guardWakesUp;
                break;

            case noGuard:
            default:
                assert(false);  // unreachable
        }
    }


    // PART 1
    // find the guard with the most sleep
    auto max_guard_iter = std::max_element(guard_sleepy_times.begin(), guard_sleepy_times.end(),
            [](const decltype(guard_sleepy_times)::value_type& a, const decltype(guard_sleepy_times)::value_type& b) -> bool
            {
                return a.second.sleep_count < b.second.sleep_count;
            });

    // find the minute which was most slept on
    auto& sm = max_guard_iter->second.sleep_minutes;
    auto max_sleep_iter = std::max_element(sm.begin(), sm.end());
    long i = max_sleep_iter - sm.begin();

    long part1_answer = max_guard_iter->first * i;

    // PART 2
    // find the minute where some guard was most frequently asleep
    long most_sleepy_minute = 0;
    long most_sleepy_guard = 0;
    int most_sleep = -1;
    for (long j=0; j<minute_sleepy_guards.size(); j++)
    {
        for (auto& g : minute_sleepy_guards[j])
        {
            if (g.second > most_sleep)
            {
                most_sleep = g.second;
                most_sleepy_guard = g.first;
                most_sleepy_minute = j;
            }
        }
    }

    long part2_answer = most_sleepy_minute * most_sleepy_guard;
    return {part1_answer, part2_answer};
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty() > 0);

    event_line_parser p;
    auto events = convert_strings<struct event>(lines, p);
    std::sort(events.begin(), events.end(), time_compare_events());

    auto result = day04_solve_part1_and_2(events);
    std::cout << result.first << std::endl;
    std::cout << result.second << std::endl;
    return 0;
}

