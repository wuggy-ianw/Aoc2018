#include <cassert>
#include <iostream>
#include <unordered_set>

#include "../util/file_parsing.h"

constexpr size_t RegisterCount = 6; // must be set before the include
#include "../util/aoc_cpu.h"


constexpr size_t ip_index = 2;


const std::array<Instruction, 32> program{
        std::bind(seti, std::placeholders::_1, 123, 0, 5),
        std::bind(bani, std::placeholders::_1, 5, 456, 5),
        std::bind(eqri, std::placeholders::_1, 5, 72, 5),
        std::bind(addr, std::placeholders::_1, 5, 2, 2),
        std::bind(seti, std::placeholders::_1, 0, 0, 2),
        std::bind(seti, std::placeholders::_1, 0, 9, 5),
        std::bind(bori, std::placeholders::_1, 5, 65536, 3),
        std::bind(seti, std::placeholders::_1, 7586220, 4, 5),
        std::bind(bani, std::placeholders::_1, 3, 255, 1),
        std::bind(addr, std::placeholders::_1, 5, 1, 5),
        std::bind(bani, std::placeholders::_1, 5, 16777215, 5),
        std::bind(muli, std::placeholders::_1, 5, 65899, 5),
        std::bind(bani, std::placeholders::_1, 5, 16777215, 5),
        std::bind(gtir, std::placeholders::_1, 256, 3, 1),
        std::bind(addr, std::placeholders::_1, 1, 2, 2),
        std::bind(addi, std::placeholders::_1, 2, 1, 2),
        std::bind(seti, std::placeholders::_1, 27, 9, 2),
        std::bind(seti, std::placeholders::_1, 0, 9, 1),
        std::bind(addi, std::placeholders::_1, 1, 1, 4),
        std::bind(muli, std::placeholders::_1, 4, 256, 4),
        std::bind(gtrr, std::placeholders::_1, 4, 3, 4),
        std::bind(addr, std::placeholders::_1, 4, 2, 2),
        std::bind(addi, std::placeholders::_1, 2, 1, 2),
        std::bind(seti, std::placeholders::_1, 25, 4, 2),
        std::bind(addi, std::placeholders::_1, 1, 1, 1),
        std::bind(seti, std::placeholders::_1, 17, 2, 2),
        std::bind(setr, std::placeholders::_1, 1, 6, 3),
        std::bind(seti, std::placeholders::_1, 7, 8, 2),
        std::bind(eqrr, std::placeholders::_1, 5, 0, 1),
        std::bind(addr, std::placeholders::_1, 1, 2, 2),
        std::bind(seti, std::placeholders::_1, 5, 0, 2)
};

using StateSet = std::unordered_set<Registers>;

void run_until_halt_or_stopped(Registers& r, RegisterType& ip, const std::function<bool(const Registers&, const RegisterType&, size_t)>& stop_condition)
{
    size_t n_instructions = 0;

    while (ip >= 0 && ip < program.size())
    {
        if (stop_condition(r, ip, n_instructions)) break;

        r[ip_index] = ip;

        // shortcut for the inner-loop, that expensively does a right-shift!
        if (ip==17)
        {
            r[4] = 1;
            r[1] = r[3] >> 8;
            r[3] = r[1];
            ip = 27;

            n_instructions += (7 * r[1]) + 7;
        }
        else
        {
            program[ip](r);

            ip = r[ip_index];
            ++ip;

            ++n_instructions;
        }
    }
}

int day21_solve_part1()
{
    Registers r{};
    RegisterType ip = 0;

    run_until_halt_or_stopped(r, ip, [](const Registers& reg, const RegisterType& cip, size_t ni)->bool
        {
            return (cip==29);    // stop at the first attempted comparison with r0
        });

    return r[5];    // program will halt earliest if r0 == r5 at the first comparison
}

int day21_solve_part2()
{
    Registers r{};
    RegisterType ip = 0;

    std::unordered_set<RegisterType> halters;
    RegisterType last_halter = 0;

    run_until_halt_or_stopped(r, ip, [&halters, &last_halter](const Registers& reg, const RegisterType& cip, size_t ni)->bool
    {
        if (cip==29)
        {
//            std::cout << reg[5] << '\t' << ni << std::endl;

            // the value in r[5] is a possible stopping condition for r0
            // if it's the same as an existing value, then we're cycling and we should just stop!
            auto insert_result = halters.insert(reg[5]);
            if (!insert_result.second)
            {
                return true;
            }

            // keep the most recently inserted
            last_halter = reg[5];
        }

        return false;
    });

    return last_halter;
}

int main()
{
    std::cout << day21_solve_part1() << std::endl;
    std::cout << day21_solve_part2() << std::endl;
    return 0;
}

