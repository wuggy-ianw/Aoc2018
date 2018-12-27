#include <cassert>
#include <iostream>
#include <functional>

#include "../util/file_parsing.h"


constexpr size_t RegisterCount = 6; // must be set before the include
#include "../util/aoc_cpu.h"


constexpr size_t ip_index = 4;


const std::array<Instruction, 36> program {
        std::bind(addi, std::placeholders::_1, 4, 16, 4),
        std::bind(seti, std::placeholders::_1, 1, 9, 5),
        std::bind(seti, std::placeholders::_1, 1, 5, 2),
        std::bind(mulr, std::placeholders::_1, 5, 2, 1),
        std::bind(eqrr, std::placeholders::_1, 1, 3, 1),
        std::bind(addr, std::placeholders::_1, 1, 4, 4),
        std::bind(addi, std::placeholders::_1, 4, 1, 4),
        std::bind(addr, std::placeholders::_1, 5, 0, 0),
        std::bind(addi, std::placeholders::_1, 2, 1, 2),
        std::bind(gtrr, std::placeholders::_1, 2, 3, 1),
        std::bind(addr, std::placeholders::_1, 4, 1, 4),
        std::bind(seti, std::placeholders::_1, 2, 6, 4),
        std::bind(addi, std::placeholders::_1, 5, 1, 5),
        std::bind(gtrr, std::placeholders::_1, 5, 3, 1),
        std::bind(addr, std::placeholders::_1, 1, 4, 4),
        std::bind(seti, std::placeholders::_1, 1, 2, 4),
        std::bind(mulr, std::placeholders::_1, 4, 4, 4),
        std::bind(addi, std::placeholders::_1, 3, 2, 3),
        std::bind(mulr, std::placeholders::_1, 3, 3, 3),
        std::bind(mulr, std::placeholders::_1, 4, 3, 3),
        std::bind(muli, std::placeholders::_1, 3, 11, 3),
        std::bind(addi, std::placeholders::_1, 1, 5, 1),
        std::bind(mulr, std::placeholders::_1, 1, 4, 1),
        std::bind(addi, std::placeholders::_1, 1, 2, 1),
        std::bind(addr, std::placeholders::_1, 3, 1, 3),
        std::bind(addr, std::placeholders::_1, 4, 0, 4),
        std::bind(seti, std::placeholders::_1, 0, 2, 4),
        std::bind(setr, std::placeholders::_1, 4, 8, 1),
        std::bind(mulr, std::placeholders::_1, 1, 4, 1),
        std::bind(addr, std::placeholders::_1, 4, 1, 1),
        std::bind(mulr, std::placeholders::_1, 4, 1, 1),
        std::bind(muli, std::placeholders::_1, 1, 14, 1),
        std::bind(mulr, std::placeholders::_1, 1, 4, 1),
        std::bind(addr, std::placeholders::_1, 3, 1, 3),
        std::bind(seti, std::placeholders::_1, 0, 0, 0),
        std::bind(seti, std::placeholders::_1, 0, 2, 4)
};

void run_until_halt(Registers& r, RegisterType& ip)
{
    while (ip >= 0 && ip < program.size())
    {
        r[ip_index] = ip;
        program[ip](r);
        ip = r[ip_index];
        ++ip;
    }
}


RegisterType day19_solve_part1()
{
    Registers registers{};
    RegisterType ip = 0;

    run_until_halt(registers, ip);

    return registers[0];
}

size_t day19_solve_part2()
{
    // For the 2nd part, this program is:
    //   initialisation to:
    //     r0 = 0
    //     r1 = 10550400
    //     r2 = 0
    //     r3 = 10551348
    //     r4 = A01 (ip)
    //     r5 = 0
    // and the program is effectively:
    //     r5 = 1
    //     do
    //       r2 = 1
    //       do
    //         if (r3 == r5 * r2) r0 += r5;
    //         r2 += 1
    //       while(r2 <= r3)
    //       r5 += 1
    //     while(r5 <= r3)
    //     HALT
    // Which effectively sums the integer divisors of 10551348 together (including 1 and 10551348)
    size_t r3 = 10551348;
    size_t r0 = 0;
    for (size_t i = 1; i<= r3; i++)
    {
        if (r3 % i == 0) r0 += i;
    }

    return r0;
}


int main()
{
    std::cout << day19_solve_part1() << std::endl;
    std::cout << day19_solve_part2() << std::endl;
    return 0;
}

