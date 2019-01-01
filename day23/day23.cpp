#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>

#include "../util/file_parsing.h"

using Ordinate = int64_t;
using Point3d = std::array<Ordinate, 3>;

struct Bot
{
    Point3d point;
    Ordinate r=0;
};

Bot parse_bot(const std::string& s)
{
    std::stringstream ss(s);
    Bot b;

    ss >> "pos=<" >> b.point[0] >> ',' >> b.point[1] >> ',' >> b.point[2] >> ">," >> "r=" >> b.r;
    assert(ss);

    return b;
}


Ordinate dist(const Point3d& a, const Point3d& b)
{
    return std::abs(a[0]-b[0]) + std::abs(a[1]-b[1]) + std::abs(a[2]-b[2]);
}

Ordinate dist_from_origin(const Point3d& a)
{
    return std::abs(a[0]) + std::abs(a[1]) + std::abs(a[2]);
}


Ordinate day23_solve_part1(const std::vector<Bot>& bots)
{
    const Bot& biggest_ranged_bot = *std::max_element(bots.begin(), bots.end(), [](const Bot& a, const Bot& b)->bool { return a.r < b.r; });
    Ordinate count = std::count_if(bots.begin(), bots.end(), [&biggest_ranged_bot](const Bot& o)->bool{
        Ordinate d = dist(o.point, biggest_ranged_bot.point);
        return d <= biggest_ranged_bot.r;
    });

    return count;
}


struct Bounds
{
    Point3d min{};
    Point3d max{};

    Point3d range{};
    Ordinate max_range = 0;
};

struct Random
{
    std::mt19937 rnd;

    Random() : rnd() {};        // use the default seed so we're deterministic
};

Bounds compute_bounds(const std::vector<Bot>& bots)
{
    constexpr Ordinate omin = std::numeric_limits<Ordinate>::min();
    constexpr Ordinate omax = std::numeric_limits<Ordinate>::max();

    // find the min and max of the cuboid containing all the bots
    Bounds bounds{{omax, omax, omax}, {omin, omin, omin}};

    for(const auto b : bots)
    {
        for(size_t i=0; i<bounds.min.size(); ++i)
        {
            bounds.min[i] = std::min(bounds.min[i], b.point[i]);
            bounds.max[i] = std::max(bounds.max[i], b.point[i]);
        }
    }

    // set the range and max_range
    for(size_t i=0; i<bounds.min.size(); ++i)
    {
        bounds.range[i] = bounds.max[i] - bounds.min[i] + 1;    // min/max is inclusive!
        bounds.max_range = std::max(bounds.max_range, bounds.range[i]);
    }

    return bounds;
}

using ScoreType = long;

struct Particle
{
    Point3d point{};
    Ordinate step_size = 0;

    ScoreType score = -1;     // -ive is 'unset', so we don't need to recompute score for particles that already are done

    Particle clone(Random& r)
    {
        // pick a random point from our point within our step_size cube
        std::uniform_int_distribution<Ordinate> sd(-step_size, step_size);

        return {{point[0]+sd(r.rnd), point[1]+sd(r.rnd), point[2]+sd(r.rnd)}, step_size};
    }

    bool operator<(const Particle& other)
    {
        // should only be comparing particles which have the score computed upon them!
        assert(score>=0);
        assert(other.score>=0);

        // order so that better scores come first
        if (score > other.score) return true;
        if (score == other.score)
        {
            // if the scores are the same, pick 'us' if we're nearer the origin
            const Ordinate our_dist = dist_from_origin(point);
            const Ordinate other_dist = dist_from_origin(other.point);

            return (our_dist < other_dist);
        }

        return false;
    }
};

using ParticleVec = std::vector<Particle>;

void append_random_particles(ParticleVec& pv, size_t n, const Bounds& bounds, Random& r)
{
    std::uniform_int_distribution<Ordinate> xd(bounds.min[0], bounds.max[0]);
    std::uniform_int_distribution<Ordinate> yd(bounds.min[1], bounds.max[1]);
    std::uniform_int_distribution<Ordinate> zd(bounds.min[2], bounds.max[2]);

    const Ordinate step_size = bounds.max_range / 256;      // use an step size that's 'pretty big'
    for(size_t i = 0; i<n; ++i)
    {
        Particle p{ {xd(r.rnd), yd(r.rnd), zd(r.rnd)}, step_size};
        pv.push_back(p);
    }
}

void score_particle(Particle& p, const std::vector<Bot>& bots)
{
    if (p.score >= 0) return;   // already scored

    auto count = std::count_if(bots.begin(), bots.end(), [&p](const Bot& bot)->bool{
        Ordinate d = dist(bot.point, p.point);
        return d <= bot.r;
    });

    p.score = count;
}

void score_particles(ParticleVec& pv, const std::vector<Bot>& bots)
{
    for (auto& particle : pv) score_particle(particle, bots);
}

void clone_particles(ParticleVec& pv, size_t n, Random& r)
{
    size_t select_limit = pv.size();

    size_t s=0;
    for(size_t i=0; i<n; ++i)
    {
        pv.push_back(pv[s].clone(r));

        ++s;
        if (s==select_limit) s=0;
    }
}

Ordinate day23_solve_part2(const std::vector<Bot>& bots)
{
    Random random;
    auto bounds = compute_bounds(bots);

    constexpr size_t n_particles = 2000;        // total number of particles
    constexpr size_t n_fresh_particles = 200;   // number of 'fresh' particles each iteration
    constexpr size_t n_best_to_keep = 600;      // the number to survive each iteration (best scorers win)
    constexpr size_t n_cloned_from_kept = 1200;  // the number of particles 'cloned' from random survivors

    constexpr size_t no_improvement_limit = 1000;

    // make a collection of random particles
    ParticleVec particles;
    append_random_particles(particles, n_particles, bounds, random);

    // set the initial scores and sort by score
    score_particles(particles, bots);
    std::sort(particles.begin(), particles.end());

    Particle all_time_best = particles[0];
    size_t iterations_with_no_improvement = 0;

    // keep going until we don't see any improvement
    while(iterations_with_no_improvement < no_improvement_limit)
    {
        // kill the bad particles
        particles.resize(n_best_to_keep);

        // clone from the kept (evenly)
        clone_particles(particles, n_cloned_from_kept, random);

        // reduce the step_size of our kept so future children explore the nearer region
        for(size_t i=0; i<n_best_to_keep; ++i)
        {
            particles[i].step_size >>= 1;
            if (particles[i].step_size == 0) particles[i].step_size = 1;
        }

        // add some fresh particles
        append_random_particles(particles, n_fresh_particles, bounds, random);

        // score the particles and sort by score
        score_particles(particles, bots);
        std::sort(particles.begin(), particles.end());

        // check if we have any improvement
        if (particles[0] < all_time_best)       // C++ hacky mess: less-than-operator is true if (a better than b)
        {
            all_time_best = particles[0];
            iterations_with_no_improvement = 0;     // reset!
        }
        else
        {
            ++iterations_with_no_improvement;
        }
    }

    return dist_from_origin(all_time_best.point);
}


int main()
{
    auto file_text = read_file("input.txt");
    assert(!file_text.empty());

    auto lines = parse_lines(file_text);
    assert(!lines.empty());

    auto bots = convert_strings<Bot>(lines, parse_bot);
    assert(!bots.empty());

    std::cout << day23_solve_part1(bots) << std::endl;
    std::cout << day23_solve_part2(bots) << std::endl;
    return 0;
}

