#include <iostream>
#include <list>
#include <map>
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

auto explore(map const & valves, int time = 30, int flow = 0, name_t current = "AA", std::set<name_t> opened = {}, std::set<name_t> visited = {}) -> std::set<int>
{
    //std::cout << "Exploring from " << current << " with time " << time << " and current flow " << flow << std::endl;
    std::set<int> ret{flow};
    if(time < 1)
        return ret;

    if(visited.contains(current))
        return ret;
    visited.insert(current);

    auto found = valves.find(current);
    if(found == valves.end())
        return ret;

    auto & next = found->second.next;

    std::for_each(next.begin(), next.end(),[current, &valves, time, flow, &opened, &visited, &ret](auto dest)
    {
        auto these = explore(valves, time-1, flow, dest, opened, visited);
        //std::cout << "Without opening " << current << " we found " << these.size() << " options" << std::endl;
        ret.insert(these.begin(), these.end());
    });
    
    if(!opened.contains(current))
    {
        // Can choose to open this valve
        --time; // It takes one minute to open the valve

        flow += found->second.rate * time; // Add this valve's contribution to the flow

        std::cout << "Opening valve " << current << " with remaining time " << time << " gives current flow " << flow << std::endl;

        ret.insert(flow);

        opened.insert(current); // We can't open it twice

        std::for_each(next.begin(), next.end(),[current, &valves, time, flow, &opened, &visited, &ret](auto dest)
        {
            auto these = explore(valves, time-1, flow, dest, opened, visited);
            //std::cout << "With opening " << current << " we found " << these.size() << " options" << std::endl;
            ret.insert(these.begin(), these.end());
        });
    }

    //std::cout << "After exploring " << current << " we found " << ret.size() << " options" << std::endl;
    return ret;
} 

int main()
{
    auto valves = parse();
    auto possible = explore(valves);

    std::cout << *possible.rbegin() << std::endl;

    return 0;
}