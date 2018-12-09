#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "../util/file_parsing.h"

struct task_set
{
    static constexpr char min_task = 'A';
    static constexpr char max_task = 'Z' + 1;
    static constexpr int n_tasks = max_task - min_task;

    std::array<bool, n_tasks> tasks = {};
    int count = 0;

    static int task_to_index(char c)
    {
        assert(c >= min_task);
        assert(c < max_task);

        return c - min_task;;
    }

    static char index_to_task(int i)
    {
        assert(i>=0);
        assert(i < n_tasks);

        return static_cast<char>(min_task + i);
    }

    bool set(char c)
    {
        int i = task_to_index(c);
        if (!tasks[i])
        {
            tasks[i] = true;
            ++count;

            return true;
        }

        return false;
    }

    bool clear(char c)
    {
        int i = task_to_index(c);
        if (tasks[i])
        {
            tasks[i] = false;
            --count;

            return true;
        }

        return false;
    }

    char first()
    {
        assert(count > 0);
        for (int i = 0; i<n_tasks; i++)
        {
            if (tasks[i]) return index_to_task(i);
        }
        assert(false);  // unreachable!
        return 0;
    }
};

std::string day07_solve_part1(std::unordered_map<char, task_set> dependencies)
{
    std::string ordered_tasks;
    size_t task_count = dependencies.size();

    // find the tasks with no dependencies, these become our runnable set
    struct task_set runnable;
    for (auto &d : dependencies)
    {
        if (d.second.count == 0)
        {
            runnable.set(d.first);
        }
    }
    assert(runnable.count > 0);    // should be at least one runnable!

    // take the first runnable, 'run it', and exclude it from all the dependencies
    // if a task has no more pending dependencies, it becomes runnable
    // keep going until everything has been ran (or we stall!)
    while (runnable.count)
    {
        char running = runnable.first();

        runnable.clear(running);
        ordered_tasks.append({running});

        for(auto &d : dependencies)
        {
           bool did_clear = d.second.clear(running);
           if (did_clear && d.second.count == 0)
           {
               // we have removed the last dependency, so we're now runnable!
               runnable.set(d.first);
           }
        }
    }
    assert(ordered_tasks.size() == task_count);     // some tasks didn't run?

    return ordered_tasks;
}

int day07_solve_part2(std::unordered_map<char, task_set> dependencies)
{
    size_t task_count = dependencies.size();

    // find the tasks with no dependencies, these become our runnable set
    struct task_set runnable;
    for (auto &d : dependencies)
    {
        if (d.second.count == 0)
        {
            runnable.set(d.first);
        }
    }
    assert(runnable.count > 0);    // should be at least one runnable!

    // we have 5 workers... they're currently working on nothing
    static constexpr int n_workers = 5;
    std::array<int, n_workers> worker_busy_until = {0};
    std::array<char, n_workers> worker_on_task = {0};

    // take the first runnable, 'run it', and exclude it from all the dependencies
    // if a task has no more pending dependencies, it becomes runnable
    // keep going until everything has been ran (or we stall!)
    int time = 0;
    int busy_workers = 0;
    while (runnable.count || busy_workers != 0)
    {
        // if a worker has completed a task, complete it
        for(int w = 0; w < n_workers; ++w)
        {
            // complete any tasks the workers have completed
            if (worker_busy_until[w] <= time && worker_on_task[w])
            {
                char running = worker_on_task[w];
                worker_on_task[w] = 0;  // not on any task now
                busy_workers --;

                for (auto &d : dependencies)
                {
                    bool did_clear = d.second.clear(running);
                    if (did_clear && d.second.count == 0)
                    {
                        // we have removed the last dependency, so we're now runnable!
                        runnable.set(d.first);
                    }
                }
            }
        }

        // now, if a worker is free, assign a new task
        for(int w = 0; w < n_workers; ++w)
        {
            if (worker_on_task[w] == 0 && runnable.count > 0)
            {
                // this worker does the first runnable task
                char running = runnable.first();
                runnable.clear(running);

                worker_busy_until[w] = time + 61 + task_set::task_to_index(running);
                worker_on_task[w] = running;
                busy_workers++;
            }
        }

        // if there are no busy workers, and no runnable tasks, we're done?
        // don't skip time forward
        if (busy_workers || runnable.count)
        {


            // skip time forward until our earliest busy worker is free
            int earliest_worker_free = std::numeric_limits<int>::max();
            for (int w = 0; w < n_workers; ++w)
            {
                if (worker_on_task[w]) earliest_worker_free = std::min(earliest_worker_free, worker_busy_until[w]);
            }

            assert(earliest_worker_free != std::numeric_limits<int>::max());

            time = earliest_worker_free;
        }
    }

    return time;
}

int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty() > 0);

    std::unordered_map<char, task_set> dependencies;
    for (const auto& s : lines)
    {
        std::stringstream ss(s);

        char pretask = 0, posttask = 0;
        ss >> "Step" >> pretask >> "must" >> "be" >> "finished" >> "before" >> "step" >> posttask >> "can" >> "begin.";
        assert(ss);

        dependencies[pretask];  // access but do not use - ensures it exists!
        dependencies[posttask].set(pretask);
    }

    std::cout << day07_solve_part1(dependencies) << std::endl;
    std::cout << day07_solve_part2(dependencies) << std::endl;
    return 0;
}

