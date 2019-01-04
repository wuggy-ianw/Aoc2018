#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <vector>
#include <numeric>

enum Force
{
    frcImmuneSystem = 0,
    frcInfection
};

enum DamageType
{
    dmgNone = 0,
    dmgFire = 1,
    dmgSlashing = 2,
    dmgBludgeoning = 4,
    dmgRadiation = 8,
    dmgCold = 16
};

using DamageTypeMask = uint32_t;
using DamageUnit = int64_t;

struct Group
{
    Force force;
    int id;
    DamageUnit units;
    DamageUnit hp;
    DamageTypeMask immunities;
    DamageTypeMask weaknesses;
    DamageUnit attack;
    DamageType inflicted;
    int initiative;

    Group* attacking = nullptr;
    Group* attackedby = nullptr;
};


using GroupContainer =  std::vector<Group>;
using GroupPtrContainer =  std::vector<Group*>;

const GroupContainer example_groups
    {
            //Immune System:
            // 17 units each with 5390 hit points (weak to radiation, bludgeoning) with an attack that does 4507 fire damage at initiative 2
            Group{frcImmuneSystem, 1, 17, 5390, dmgNone, dmgRadiation | dmgBludgeoning, 4507, dmgFire, 2},
            // 989 units each with 1274 hit points (immune to fire; weak to bludgeoning, slashing) with an attack that does 25 slashing damage at initiative 3
            Group{frcImmuneSystem, 2, 989, 1274, dmgFire, dmgBludgeoning | dmgSlashing, 25, dmgSlashing, 3},

            // Infection:
            // 801 units each with 4706 hit points (weak to radiation) with an attack that does 116 bludgeoning damage at initiative 1
            Group{frcInfection, 1, 801, 4706, dmgNone, dmgRadiation, 116, dmgBludgeoning, 1},
            // 4485 units each with 2961 hit points (immune to radiation; weak to fire, cold) with an attack that does 12 slashing damage at initiative 4
            Group{frcInfection, 2, 4485, 2961, dmgRadiation, dmgFire | dmgCold, 12, dmgSlashing, 4}
    };

const GroupContainer input_groups
{       // Immune System:
        // 2991 units each with 8084 hit points (weak to fire) with an attack that does 19 radiation damage at initiative 11
        Group{frcImmuneSystem, 1, 2991, 8084, dmgNone, dmgFire, 19, dmgRadiation, 11},
        // 4513 units each with 3901 hit points (weak to slashing; immune to bludgeoning, radiation) with an attack that does 7 bludgeoning damage at initiative 12
        Group{frcImmuneSystem, 2, 4513, 3901, dmgBludgeoning | dmgRadiation, dmgSlashing, 7, dmgBludgeoning, 12},
        // 5007 units each with 9502 hit points (immune to bludgeoning; weak to fire) with an attack that does 16 fire damage at initiative 2
        Group{frcImmuneSystem, 3, 5007, 9502, dmgBludgeoning, dmgFire, 16, dmgFire, 2},
        // 2007 units each with 5188 hit points (weak to radiation) with an attack that does 23 cold damage at initiative 9
        Group{frcImmuneSystem, 4, 2007, 5188, dmgNone, dmgRadiation, 23, dmgCold, 9},
        // 1680 units each with 1873 hit points (immune to bludgeoning; weak to radiation) with an attack that does 10 bludgeoning damage at initiative 10
        Group{frcImmuneSystem, 5, 1680, 1873, dmgBludgeoning, dmgRadiation, 10, dmgBludgeoning, 10},
        // 1344 units each with 9093 hit points (immune to bludgeoning, cold; weak to radiation) with an attack that does 63 cold damage at initiative 16
        Group{frcImmuneSystem, 6, 1344, 9093, dmgBludgeoning | dmgCold, dmgRadiation, 63, dmgCold, 16},
        // 498 units each with 2425 hit points (immune to fire, bludgeoning, cold) with an attack that does 44 slashing damage at initiative 3
        Group{frcImmuneSystem, 7, 498, 2425, dmgFire | dmgBludgeoning | dmgCold, dmgNone, 44, dmgSlashing, 3},
        // 1166 units each with 7295 hit points with an attack that does 56 bludgeoning damage at initiative 8
        Group{frcImmuneSystem, 8, 1166, 7295, dmgNone, dmgNone, 56, dmgBludgeoning, 8},
        // 613 units each with 13254 hit points (immune to radiation, cold, fire) with an attack that does 162 radiation damage at initiative 15
        Group{frcImmuneSystem, 9, 613, 13254, dmgRadiation | dmgCold | dmgFire, dmgNone, 162, dmgRadiation, 15},
        // 1431 units each with 2848 hit points (weak to radiation) with an attack that does 19 cold damage at initiative 1
        Group{frcImmuneSystem, 10, 1431, 2848, dmgNone, dmgRadiation, 19, dmgCold, 1},

        // Infection:
        // 700 units each with 47055 hit points (weak to fire; immune to slashing) with an attack that does 116 fire damage at initiative 14
        Group{frcInfection, 1, 700, 47055, dmgSlashing, dmgFire, 116, dmgFire, 14},
        // 2654 units each with 13093 hit points (weak to radiation) with an attack that does 8 radiation damage at initiative 19
        Group{frcInfection, 2, 2654, 13093, dmgNone, dmgRadiation, 8, dmgRadiation, 19},
        // 5513 units each with 18026 hit points (immune to radiation; weak to slashing) with an attack that does 6 slashing damage at initiative 20
        Group{frcInfection, 3, 5513, 18026, dmgRadiation, dmgSlashing, 6, dmgSlashing, 20},
        // 89 units each with 48412 hit points (weak to cold) with an attack that does 815 radiation damage at initiative 17
        Group{frcInfection, 4, 89, 48412, dmgNone, dmgCold, 815, dmgRadiation, 17},
        // 2995 units each with 51205 hit points (weak to cold) with an attack that does 28 slashing damage at initiative 7
        Group{frcInfection, 5, 2995, 51205, dmgNone, dmgCold, 28, dmgSlashing, 7},
        // 495 units each with 21912 hit points with an attack that does 82 cold damage at initiative 13
        Group{frcInfection, 6, 495, 21912, dmgNone, dmgNone, 82, dmgCold, 13},
        // 2911 units each with 13547 hit points with an attack that does 7 slashing damage at initiative 18
        Group{frcInfection, 7, 2911, 13547, dmgNone, dmgNone, 7, dmgSlashing, 18},
        // 1017 units each with 28427 hit points (immune to fire) with an attack that does 52 fire damage at initiative 4
        Group{frcInfection, 8, 1017, 28427, dmgFire, dmgNone, 52, dmgFire, 4},
        // 2048 units each with 29191 hit points (weak to bludgeoning) with an attack that does 22 bludgeoning damage at initiative 6
        Group{frcInfection, 9, 2048, 29191, dmgNone, dmgBludgeoning, 22, dmgBludgeoning, 6},
        // 1718 units each with 15725 hit points (immune to cold) with an attack that does 18 slashing damage at initiative 5
        Group{frcInfection, 10, 1718, 15725, dmgCold, dmgNone, 18, dmgSlashing, 5}
};


GroupPtrContainer make_pointers(GroupContainer& groups)
{
    GroupPtrContainer gptrs;
    for (size_t i=0; i<groups.size(); ++i) gptrs.push_back(&groups[i]);
    return gptrs;
}


DamageUnit effective_power(const Group& g)
{
    return g.units * g.attack;
}


DamageUnit actual_damage(const Group& attacker, const Group& defender)
{
    DamageUnit base_damage = effective_power(attacker);

    DamageUnit multiplier = 1;
    if (defender.immunities & attacker.inflicted) multiplier = 0;
    if (defender.weaknesses & attacker.inflicted) multiplier = 2;

    return base_damage * multiplier;
}


DamageUnit potential_damage(const Group& attacker, const Group& defender)
{
    // if attacker and defender are on the same team, then never do damage
    if (attacker.force == defender.force) return 0;

    // if the defender is already being attacked, don't do damage (so we don't attack the same group twice)
    if (defender.attackedby) return 0;

    // if the defender is already dead, don't do damage
    if (defender.units == 0) return 0;

    return actual_damage(attacker, defender);
}



bool target_selection_evaluation_order(const Group* const a, const Group* const b)
{
    // run order is such that a before b if a has the bigger effective_power, then if the bigger initiative
    return std::make_pair(-effective_power(*a), -a->initiative) < std::make_pair(-effective_power(*b), -b->initiative);
};

bool most_damage_to_enemy_group_order(const Group& attacker, const Group& l, const Group& r)
{
    // select the group to attack
    // maximise damage, then select the group with the highest attack power, then the highest initiative
    return std::make_tuple(potential_damage(attacker, l), effective_power(l), l.initiative) <
           std::make_tuple(potential_damage(attacker, r), effective_power(r), r.initiative);
}

void set_most_damaged_enemy_group(Group& attacker, GroupContainer& groups)
{
    auto iter = std::max_element(groups.begin(), groups.end(), [&attacker](const Group& l, const Group& r)->bool{
        return most_damage_to_enemy_group_order(attacker, l, r);
   });

    // only set the attacker/attacked if we're doing non-zero damage
    if (potential_damage(attacker, *iter))
    {
        attacker.attacking = &(*iter);
        iter->attackedby = &attacker;
    }
}


void select_targets(GroupContainer& groups)
{
    // clear the attacked/attacked-by entries
    for (auto& g : groups) g.attackedby = g.attacking = nullptr;

    // work out the order to select targets
    auto group_ptrs = make_pointers(groups);
    std::sort(group_ptrs.begin(), group_ptrs.end(), target_selection_evaluation_order);

    // in that order, have the attackers pick their targets
    for (auto attacker : group_ptrs)
    {
        set_most_damaged_enemy_group(*attacker, groups);
    }
}

DamageUnit attack_targets(GroupContainer& groups)
{
    // work out the order to attack targets (decreasing initiative order)
    auto group_ptrs = make_pointers(groups);
    std::sort(group_ptrs.begin(), group_ptrs.end(), [](const Group* const a, const Group* const b)->bool{
        return a->initiative > b->initiative;
    });

    // in that order, have the attackers attack their targets
    DamageUnit total_units_killed = 0;
    for (auto attacker : group_ptrs)
    {
        if (!attacker->attacking) continue;     // not attacking, skip
        if (attacker->units == 0) continue;     // dead units can't attack

        Group& defender = *attacker->attacking;
        auto damage_done = actual_damage(*attacker, defender);
        auto units_killed = std::min(damage_done / defender.hp, defender.units);
        defender.units -= units_killed;
        total_units_killed += units_killed;
    }

    return total_units_killed;
}


std::pair<DamageUnit, DamageUnit> run_until_winner(GroupContainer& groups)
{
    // count how many units are alive on each side
    auto immune_count = std::count_if(groups.begin(), groups.end(), [](const Group& g){ return (g.force == frcImmuneSystem) && (g.units != 0); });
    auto infection_count = std::count_if(groups.begin(), groups.end(), [](const Group& g){ return (g.force == frcInfection) && (g.units != 0); });

    while(immune_count != 0 && infection_count != 0)
    {
        // have groups select their targets for this round
        select_targets(groups);

        // have groups perform their attacks!
        auto units_killed = attack_targets(groups);

        // (re)count how many units are alive on each side
        immune_count = std::count_if(groups.begin(), groups.end(), [](const Group& g){ return (g.force == frcImmuneSystem) && (g.units != 0); });
        infection_count = std::count_if(groups.begin(), groups.end(), [](const Group& g){ return (g.force == frcInfection) && (g.units != 0); });

        // if we did an iteration without killing any units, then we're stalled
        if (units_killed == 0) break;
    }

    return {immune_count, infection_count};
}

DamageUnit day24_solve_part1(const GroupContainer& initial_groups)
{
    auto groups = initial_groups;
    run_until_winner(groups);

    // count the number of units remaining...
    DamageUnit units = 0;
    for (auto& g : groups) units += g.units;

    return units;
}

DamageUnit day24_solve_part2(const GroupContainer& initial_groups)
{
    // linearly search giving the immune system a boost
    DamageUnit boost = 0;

    GroupContainer groups;
    while(true)
    {
        groups = initial_groups;
        for (auto& g : groups)
        {
            if (g.force == frcImmuneSystem) g.attack += boost;
        }

        auto result = run_until_winner(groups);
        if(result.first > 0 && result.second == 0) break;     // the immune system won!

        ++boost;
    }

    // count the number of units remaining...
    DamageUnit units = 0;
    for (auto& g : groups) units += g.units;

    return units;
}


int main()
{
    std::cout << day24_solve_part1(input_groups) << std::endl;
    std::cout << day24_solve_part2(input_groups) << std::endl;
    return 0;
}

