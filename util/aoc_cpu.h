//
// Created by wuggy on 26/12/18.
//

#ifndef AOC2018_AOC_CPU_H
#define AOC2018_AOC_CPU_H

using RegisterType = int;

using Registers = std::array<RegisterType, RegisterCount>;
using Instruction = std::function<void(Registers&)>;        // usually made with std::bind on an InstructionFunc
using InstructionFunc = std::function<void(Registers&, RegisterType, RegisterType, RegisterType)>;

template<typename OpType>
void inst_r(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = OpType()(r[a], r[b]);
}

template<typename OpType>
void inst_i(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = OpType()(r[a], b);
}

auto addr = inst_r<std::plus<RegisterType>>;
auto addi = inst_i<std::plus<RegisterType>>;

auto mulr = inst_r<std::multiplies<RegisterType>>;
auto muli = inst_i<std::multiplies<RegisterType>>;

auto banr = inst_r<std::bit_and<RegisterType>>;
auto bani = inst_i<std::bit_and<RegisterType>>;

auto borr = inst_r<std::bit_or<RegisterType>>;
auto bori = inst_i<std::bit_or<RegisterType>>;

void setr(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = r[a];
}

void seti(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = a;
}


void gtir(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (a > r[b]) ? 1 : 0;
}

void gtri(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (r[a] > b) ? 1 : 0;
}

void gtrr(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (r[a] > r[b]) ? 1 : 0;
}


void eqir(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (a == r[b]) ? 1 : 0;
}

void eqri(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (r[a] == b) ? 1 : 0;
}

void eqrr(Registers& r, RegisterType a, RegisterType b, RegisterType c)
{
    r[c] = (r[a] == r[b]) ? 1 : 0;
}


#endif //AOC2018_AOC_CPU_H
