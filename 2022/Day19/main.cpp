#include <array>
#include <iostream>
#include <vector>
#include <regex>
#include <sstream>
#include <string>

static std::size_t num_rounds = 24;

using costs_t = std::array<int, 4>;

struct blueprint_t
{
public:
    blueprint_t(costs_t robot_cost_ore,
                costs_t robot_cost_clay,
                costs_t robot_cost_obsidian,
                costs_t robot_cost_geode)
    : m_costs{robot_cost_ore, robot_cost_clay, robot_cost_obsidian, robot_cost_geode},
      m_max_robots{max_robots(m_costs)}
    /*m_cost_ore{robot_cost_ore},
      m_cost_clay{robot_cost_clay},
      m_cost_obsidian{robot_cost_clay},
      m_cost_geode{robot_cost_geode}*/
    {
        /*std::cout << "Constructing blueprint. Max robots of each type: ";

        for(int type = 0; type < 3; ++type)
            std::cout << m_max_robots[type] <<".";
        
        std::cout << std::endl;

        std::cout << "Costs: " << std::endl;
        for(int robot = 0; robot < 4; ++robot)
        {
            for(int type = 0; type < 4; ++type)
            {
                std::cout << m_costs[robot][type] << " ";
            }
            std::cout << std::endl;
        }*/

        m_previous_best.fill(0);
    }

    auto max_geodes(std::size_t turn = 0) const -> std::size_t;

private:
    using state_t = std::pair<costs_t, costs_t>;

    static auto print(state_t const & state) -> std::string;

    static constexpr costs_t starting_robots{1,0,0,0};
    static constexpr costs_t starting_minerals{0,0,0,0};
    static constexpr state_t starting_state{std::make_pair(starting_robots, starting_minerals)};

    static auto max_robots(const std::array<costs_t, 4> costs) -> costs_t;
    auto max_geodes_p(std::size_t turn, state_t state) const -> std::size_t;
    auto build_robot(std::size_t turn, state_t state, int type) const -> std::size_t;
    auto enough_minerals(const costs_t cost, state_t const & state) const -> bool;

    const std::array<costs_t, 4> m_costs;
    const costs_t m_max_robots;
    mutable std::array<int, 33> m_previous_best;

    /*const costs_t m_cost_ore;
    const costs_t m_cost_clay;
    const costs_t m_cost_obsidian;
    const costs_t m_cost_geode;*/
};

auto blueprint_t::print(state_t const & state) -> std::string
{
    std::ostringstream o;

    for(int i = 0; i < 4; ++i)
        o << state.first[i] << ".";
    for(int i = 0; i < 4; ++i)
        o << state.second[i] << ".";

    return o.str();
}

// Figure out the maximum sensible number of each robot
auto blueprint_t::max_robots(const std::array<costs_t, 4> costs) -> costs_t
{
    costs_t max_each_robot;
    // For each mineral type, we don't want more robots of that type than we can use in a turn
    for(int type = 0; type < 4; ++type)
    {
        int max = 0;
        for(int robot = 0; robot < 4; ++robot)
        {
            max = std::max(max, costs[robot][type]);
        }
        max_each_robot[type] = max;
    }

    max_each_robot.back() = std::numeric_limits<int>::max();

    return max_each_robot;
}

auto blueprint_t::max_geodes(std::size_t turn) const -> std::size_t
{
    return max_geodes_p(turn, starting_state);
}

auto blueprint_t::max_geodes_p(std::size_t turn, state_t state) const -> std::size_t
{
    // Spend for robots has to be upfront, before earnings this round
    std::array<bool, 4> affordable;
    for(int type = 3; type >= 0; --type)
        affordable[type] = enough_minerals(m_costs[type], state);

    // All robots get more minerals
    for(int i = 0; i < 4; ++i)
        state.second[i] += state.first[i];
    ++turn;

    auto & geodes = state.second.back();

    // If we don't have even half of the best we've managed by this point, abandon
    if(turn < m_previous_best.size())
    {
        auto & prev = m_previous_best[turn];
        if(geodes > prev)
        {
            //std::cout << "New record for most geodes: " << geodes << " (previous was " << prev << ") with turn = " << turn << " state = " << print(state) << std::endl;
            prev = geodes;
        }
        auto minimum = prev / 2;
        if(geodes < minimum)
            return 0;
    }

    if(turn == num_rounds)
        return geodes;
    
    std::size_t best = 0;

    for(int type = 3; type >= 0; --type)
        // Only make a robot of this type if we've not hit the sensible limit yet and we can afford to
        if(state.first[type] < m_max_robots[type] && affordable[type])
            best = std::max(best, build_robot(turn, state, type));

    // Always consider doing nothing for a turn
    return std::max(best, max_geodes_p(turn, state));
}

auto blueprint_t::build_robot(std::size_t turn, state_t state, int type) const -> std::size_t
{
    ++state.first[type];
    for(int i = 0; i < 4; ++i)
    {
        state.second[i] -= m_costs[type][i];

        if(state.second[i] < 0)
        {
            std::cout << "Built a robot and ended up with negative quantity of type " << i << std::endl;
            throw 5;
        }
    }

    //if(type == 2)
        //std::cout << "Built robot of type " << type << " turn = " << turn << " state = " << print(state) << std::endl;

    return max_geodes_p(turn, state);
}

auto blueprint_t::enough_minerals(const costs_t cost, state_t const & state) const -> bool
{
    bool enough = true;
    for(int i = 0; i < 4; ++i)
        enough = enough && state.second[i] >= cost[i];
    return enough;
}

using blueprint_vector = std::vector<blueprint_t>;

auto parse_blueprints() -> blueprint_vector
{
    blueprint_vector ret;
    std::string line;
    std::smatch match;
    std::regex regex{"Blueprint ([0-9]+): Each ore robot costs ([0-9]+) ore. Each clay robot costs ([0-9]+) ore. Each obsidian robot costs ([0-9]+) ore and ([0-9]+) clay. Each geode robot costs ([0-9]+) ore and ([0-9]+) obsidian."};

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, match, regex))
        {
            throw 5;
        }

        costs_t ore_costs{stoi(match[2]), 0, 0};
        costs_t clay_costs{stoi(match[3]), 0, 0};
        costs_t obsidian_costs{stoi(match[4]), stoi(match[5]), 0};
        costs_t geode_costs{stoi(match[6]), 0, stoi(match[7])};

        ret.emplace_back(blueprint_t{ore_costs, clay_costs, obsidian_costs, geode_costs});
    }

    return ret;
}

auto part1(blueprint_vector const & blueprints) -> std::size_t
{
    std::size_t best = 0;

    for(int i = 0; i < blueprints.size(); ++i)
    {
        auto res = blueprints[i].max_geodes();

        best += res * (i+1);

        std::cout << "Blueprint " << i << " managed a record of " << res << std::endl;
    }

    return best;
}

auto part2(blueprint_vector const & blueprints) -> std::size_t
{
    std::size_t product = 1;
    num_rounds = 32;

    for(int i = 0; i < 3; ++i)
    {
        auto res = blueprints[i].max_geodes();
        product *= res;
        std::cout << "Blueprint " << i << " managed a record of " << res << std::endl;
    }

    return product;
}

int main()
{
    auto blueprints = parse_blueprints();

    std::cout << part1(blueprints) << std::endl;
    std::cout << part2(blueprints) << std::endl;
    return 0;
}