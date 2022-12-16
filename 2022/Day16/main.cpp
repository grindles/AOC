#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string>

using name_t = std::string;

struct node_t
{
    int rate;
    std::list<name_t> next;
};

using map = std::map<name_t, node_t>;

auto destinations(std::string csl) -> std::list<name_t>
{
    std::list<name_t> ret;

    while(csl.length() >= 2)
    {
        ret.push_back(csl.substr(0, 2));
        csl.erase(0, 4);
    }

    return ret;
}

auto parse() -> map
{
    map ret;

    std::string line;
    std::regex regex{"^Valve ([A-Z][A-Z]) has flow rate=([0-9]+); tunnels? leads? to valves? (.*)$"};
    std::smatch match;

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, match, regex))
        {
            std::cout << "Failed to parse " << line << std::endl;
            throw 5;
        }

        name_t name = match[1];
        node_t node{stoi(match[2]), destinations(match[3])}; 

        ret.insert(std::make_pair(name, node));
    }

    return ret;
}

auto flow(map const & caves, std::set<name_t> const & opened) -> int
{
    return std::accumulate(opened.begin(), opened.end(), int{0}, [&caves](int total, auto name)
    {
        auto found = caves.find(name);
        return total + found->second.rate;
    });
}

auto explore(map const & caves, name_t loc = "AA", int time = 1, int pressure = 0, std::set<name_t> opened = {}) -> int
{
    auto current = caves.find(loc);
    auto next = current->second.next;
    int best = pressure;

    if(time > 30)
    {
        return best;
    }

    for(int i = 0; i < 2; ++i)
    {
        pressure += flow(caves, opened);

        // First time around not open
        std::for_each(next.begin(), next.end(), [&best, caves, time, &pressure, &opened](auto n)
        {
            best = std::max(best, explore(caves, n, time + 1, pressure, opened));
        });
        
        // Second time around is open
        opened.insert(loc);
        ++time;
    }

    return best;
}

int main()
{
    auto valves = parse();
    std::cout << explore(valves) << std::endl;

    return 0;
}