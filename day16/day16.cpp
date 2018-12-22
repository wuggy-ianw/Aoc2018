#include <array>
#include <cassert>
#include <iostream>
#include <functional>
#include <sstream>
#include <unordered_set>
#include <bitset>
#include <numeric>

#include "../util/file_parsing.h"

using RegisterType = int;

using Registers = std::array<RegisterType, 4>;
using Instruction = std::array<RegisterType, 4>;

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


const std::array<InstructionFunc, 16> instructions
    {
        addr, addi,
        mulr, muli,
        banr, bani,
        borr, bori,
        setr, seti,
        gtir, gtri, gtrr,
        eqir, eqri, eqrr
    };


struct Sample
{
    Registers before;
    Instruction instruction;
    Registers after;
};

Instruction parse_instruction(const std::string& l)
{
    Instruction i;

    std::stringstream ss(l);
    ss >> i[0]
       >> i[1]
       >> i[2]
       >> i[3];
    assert(ss);

    return i;

}

std::vector<Sample> parse_samples(const std::vector<std::string>& lines)
{
    std::vector<Sample> samples;

    for (auto l = lines.begin(); l != lines.end() ;)
    {
        // We expect a 'before' line, an instruction line, and an 'after' line, then a blank line!
        Sample sample = {0};

        std::stringstream before_ss(*l++);
        before_ss >> "Before:" >> '['
                  >> sample.before[0] >> ','
                  >> sample.before[1] >> ','
                  >> sample.before[2] >> ','
                  >> sample.before[3] >> ']';
        assert(before_ss);
        assert(l != lines.end());


        sample.instruction = parse_instruction(*l++);
        assert(l != lines.end());

        std::stringstream after_ss(*l++);
        after_ss >> "After:" >> '['
                  >> sample.after[0] >> ','
                  >> sample.after[1] >> ','
                  >> sample.after[2] >> ','
                  >> sample.after[3] >> ']';
        assert(after_ss);
        assert(l != lines.end());

        ++l;    // for the expected blank line
        samples.push_back(sample);
    }

    return samples;
}


int day16_solve_part1(const std::vector<Sample>& samples)
{
    // for every sample, run each instruction with the before-inputs
    // check if the after-output matches, and count the sample if it does
    int samples_matched_3_instructions = 0;

    for (auto& s : samples)
    {
        int matching_instructions = 0;
        for (auto& i : instructions)
        {
            Registers registers = s.before;
            i(registers, s.instruction[1], s.instruction[2], s.instruction[3]);

            if (registers == s.after) ++matching_instructions;
        }

        if (matching_instructions >=3 ) ++samples_matched_3_instructions;
    }

    return samples_matched_3_instructions;
}

int day16_solve_part2(const std::vector<Sample>& samples, const std::vector<Instruction>& program)
{
    // process the samples to find the matching instructions
    // can use an AND process to eliminate instructions from the set until only one remains

    // build a mapping of opcode to possible instructions, initially including every instruction
    std::unordered_map<RegisterType, std::vector<size_t>> opcode_possible_instructions;
    for(auto& s : samples)
    {
        auto insert_result = opcode_possible_instructions.insert({s.instruction[0], {}});
        if (insert_result.second)
        {
            auto& v = insert_result.first->second;

            // fill the vector with indices in order
            // (not that the order matters! :)
            v.resize(instructions.size());
            std::iota(v.begin(), v.end(), 0);
        }
    }

    // for every sample we have, exclude any non-matching instructions
    for (auto& s : samples)
    {
        auto iter = opcode_possible_instructions.find(s.instruction[0]);
        assert(iter != opcode_possible_instructions.end());     // unknown instruction?! eh?

        // remove any instruction functions which don't match this sample
        auto& v = iter->second;
        auto ri = std::remove_if(v.begin(), v.end(), [&](size_t index)->bool{
            Registers registers = s.before;
            instructions[index](registers, s.instruction[1], s.instruction[2], s.instruction[3]);
            return (registers != s.after);
        });

        v.erase(ri, v.end());
    }

    // now make the instructions unique by doing cycles excluding the known instructions
    std::unordered_map<RegisterType, InstructionFunc> opcode_instructions;
    while(opcode_instructions.size() < opcode_possible_instructions.size())
    {
        for(auto& kv : opcode_possible_instructions)
        {
            // skip if we've already set this opcode
            if (opcode_instructions.find(kv.first) != opcode_instructions.end()) continue;

            // if this is unique, make this opcode known and exclude it's index from others
            auto& v = kv.second;
            if (v.size() == 1)
            {
                // set this opcode
                size_t instruction_index = v[0];
                opcode_instructions[kv.first] = instructions[instruction_index];

                // exclude this instruction index from all other possible opcodes
                for(auto& exkv : opcode_possible_instructions)
                {
                    auto& exv = exkv.second;
                    exv.erase(std::remove(exv.begin(), exv.end(), instruction_index), exv.end());
                }
            }
        }
    }


    // opcode_possible_instructions should now have exactly one instruction per opcode... run the program
    Registers r{};
    for (auto& pi : program)
    {
        auto iter = opcode_instructions.find(pi[0]);
        assert(iter != opcode_instructions.end());     // unknown opcode?! eh?

        const auto& i = iter->second;
        i(r, pi[1], pi[2], pi[3]);
    }

    return r[0];
}


int main()
{
    auto samples_text = read_file("input.txt");
    assert(!samples_text.empty());

    auto samples_lines = parse_lines(samples_text);
    assert(!samples_lines.empty());

    auto samples = parse_samples(samples_lines);
    std::cout << day16_solve_part1(samples) << std::endl;

    auto program_text = read_file("input_instrs.txt");
    assert(!program_text.empty());

    auto program_lines = parse_lines(program_text);
    assert(!program_lines.empty());

    auto program = convert_strings<Instruction>(program_lines, parse_instruction);
    std::cout << day16_solve_part2(samples, program) << std::endl;

    return 0;
}

