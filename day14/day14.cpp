#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>


struct generator
{
    std::array<size_t, 2> elf_positions = {0, 1};
    std::vector<int> recipes = {3, 7};

    // Run one step, generating one or two new recipes
    void next()
    {
        // create and append new recipes
        int sum = 0;
        for (auto& pos : elf_positions) sum += recipes[pos];

        if (sum >= 10) recipes.push_back(1);      // sum can only be up to 18, so if it's over 10 then just append a 1
        recipes.push_back(sum % 10);

        // move the elves positions forward
        for (auto& pos : elf_positions) pos = (pos + 1 + recipes[pos]) % recipes.size();
    }
};

std::string day14_solve_part1(const size_t num_practice_recipes)
{
    // number of new recipes we need
    constexpr size_t new_recipes_needed = 10;
    const size_t recipe_limit = new_recipes_needed + num_practice_recipes;

    struct generator gen;
    while(gen.recipes.size() < recipe_limit) gen.next();

    // export the next 10 recipes after the practice recipes
    std::stringstream ss;
    for (size_t i = num_practice_recipes; i < recipe_limit; ++i) ss << gen.recipes[i];
    return ss.str();
}

long day14_solve_part2(const std::vector<int> match)
{
    struct generator gen;
    while(true)
    {
        gen.next();
        if (gen.recipes.size() < match.size()) continue;    // can't match, not enough input yet!

        // try and match one character further in (since we might have appended 2 characters)
        if (std::equal(match.begin(), match.end(), gen.recipes.end() - match.size() - 1, gen.recipes.end() - 1)) return (gen.recipes.end() - match.size() - 1) - gen.recipes.begin();

        // try and match at the end
        if (std::equal(match.begin(), match.end(), gen.recipes.end() - match.size(), gen.recipes.end())) return (gen.recipes.end() - match.size()) - gen.recipes.begin();

         // fail if we run for a very very long time
        assert(gen.recipes.size() < (1ull << 26));
    }
}

int main()
{
    std::cout << day14_solve_part1(894501) << std::endl;
    std::cout << day14_solve_part2({8, 9, 4, 5, 0, 1}) << std::endl;
    return 0;
}

