#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string>

using name_map = std::map<std::string, std::size_t>;

using name_t = std::size_t;
using neighbour_t = std::pair<int, name_t>;

struct node_t
{
    int rate;
    std::vector<neighbour_t> next;
};

using map = std::vector<node_t>;

auto destinations(std::string csl, name_map & names) -> std::vector<neighbour_t>
{
    std::vector<neighbour_t> ret;

    while(csl.length() >= 2)
    {
        auto name = csl.substr(0, 2);
        if(!names.contains(name))
            names[name] = names.size();
        auto id = names[name];
        ret.push_back(std::make_pair(1, id));
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

    name_map names;

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, match, regex))
        {
            std::cout << "Failed to parse " << line << std::endl;
            throw 5;
        }

        std::string name = match[1];

        if(!names.contains(name))
            names[name] = names.size();

        name_t id = names[name];
        node_t node{stoi(match[2]), destinations(match[3], names)};

        if(ret.size() <= id)
        {
            ret.resize(id+1);
        }

        ret[id] = node;
    }

    return ret;
}

auto shortest(name_t from, name_t to, map const & map) -> int
{
    if(from == to)
        return 0;
    //std::cout << "Finding shortest path between " << from << " and " << to << ": ";
    auto & source = map.at(from);

    // BFS from source until matching destination is reached
    std::multimap<int, name_t> queue;
    auto & dests = source.next;

    std::for_each(dests.begin(), dests.end(), [&queue](auto d)
    {
        //std::cout << "Initial queue includes " << f.second << std::endl;
        queue.insert(d);
    });

    while(!queue.empty())
    {
        auto iter = queue.begin();
        auto dist = iter->first;
        auto & next = iter->second;

        if(next == to)
        {
            // We've found it, and it must be the shortest so far
            //std::cout << dist << std::endl;
            return dist;
        }

        auto & source = map.at(next);
        auto extra_hops = source.next;

        std::for_each(extra_hops.begin(), extra_hops.end(), [&queue, dist](auto hop)
        {
            queue.insert(std::make_pair(dist + hop.first, hop.second));
        });

        queue.erase(iter);
    }

    std::cout << "Failed to find any path from " << from << " to " << to << std::endl;
    throw 7;
}

using distances = std::map<name_t, int>;

// Find shortest path from source to all other nodes
auto Dijkstra(map const & graph, name_t source) -> distances
{
    std::set<name_t> visited{};
    std::multimap<int, name_t> q{};

    auto & node = graph.at(source);
    auto & neighbours = node.next;

    std::for_each(neighbours.begin(), neighbours.end(), [&q](auto neighbour)
    {
        q.insert(neighbour);
    });

    distances dist;

    while(!q.empty())
    {
        auto iter = q.begin();

        auto distance = iter->first;
        auto name = iter->second;

        if(visited.insert(name).second)
        {
            // First time visiting this node, so add it to result if it's a new record
            auto prev = dist.find(name);
            if(prev == dist.end() || prev->second < distance)
            {
                dist.insert(std::make_pair(name, distance));
            }
            
            // Now queue up all it's neighbours
            auto & node = graph.at(name);

            auto & neighbours = node.next;

            std::for_each(neighbours.begin(), neighbours.end(), [&q, distance](auto neighbour)
            {
                // Enqueue this neighbour, adding current distance
                q.insert(std::make_pair(neighbour.first + distance, neighbour.second));
            });
        }

        q.erase(iter);
    }

    return dist;
}

// Figure out the distances between each pair of interesting nodes
auto reduce(map const & graph) -> map
{
    std::set<name_t> interesting{0}; // AA is always interesting as it's the start node

    // Filter the list of nodes to those with non-zero flow rate
    for(std::size_t id = 0; id < graph.size(); ++id)
    {
        if(graph[id].rate > 0)
        {
            interesting.insert(id);
        }
    }

    std::cout << "Found " << interesting.size() << " nodes of interest out of " << graph.size() << std::endl;

    // For each interesting node, find it's shortest path to neighbours
    map reduced;

    reduced.resize(graph.size());

    // Create mapping between each pair of points
    std::for_each(interesting.begin(), interesting.end(), [interesting, graph, &reduced](auto source)
    {
        reduced.at(source).rate = graph.at(source).rate;
        
        // New node
        auto & next = reduced.at(source).next;

        auto neighbours = Dijkstra(graph, source);

        // Only record the connections to interesting neighbours
        std::for_each(neighbours.begin(), neighbours.end(), [&next, graph, interesting](auto neighbour)
        {
            auto name = neighbour.first;
            auto distance = neighbour.second;

            if(interesting.contains(name))
            {
                next.push_back(std::make_pair(distance, name));
            }
        });
    });

    return reduced;
}

struct state_t
{
    auto operator<(state_t const & rhs) const -> bool
    {
        if(remaining_time != rhs.remaining_time)
            return remaining_time < rhs.remaining_time;
        if(location != rhs.location)
            return location < rhs.location;

        if(elephant_location != rhs.elephant_location)
            return elephant_location < rhs.elephant_location;
        if(elephant_wait != rhs.elephant_wait)
            return elephant_wait < rhs.elephant_wait;
        return opened.to_ullong() < rhs.opened.to_ullong();
    }

    void print() const
    {
        std::cout << remaining_time << " mins remaining, primary location " << location << ", elephant will be at " << elephant_location << " in " << elephant_wait << std::endl;
    }

    // Reduce the remaining time, which may switch focus between actor (human and elephant)
    // Returns true if context switched
    auto reduce_time(int mins) -> bool
    {
        if(mins > elephant_wait)
        {
            //std::cout << "Reducing remaining time by " << mins << " switches context to elephant, which only had to wait " << elephant_wait << std::endl;
            //print();
            remaining_time -= elephant_wait;
            elephant_wait = mins - elephant_wait;

            // To ensure that comparion operation finds as many matches as possible when the state is really the same, keep the context sorted when both actors are ready to go
            if(elephant_wait > 0 || location > elephant_location)
            {
                std::swap(location, elephant_location);
            }
            return true;
            //print();
        }
        else
        {
            remaining_time -= mins;
            elephant_wait -= mins;
            return false;
        }
    }

    int remaining_time;
    name_t location;
    name_t elephant_location;
    int elephant_wait;
    std::bitset<64> opened;
};

static const state_t initial_state{30, 0, 0, 30, 0};
static const state_t with_elephant{26, 0, 0, 0, 0};

using state_history = std::map<state_t, int>;

auto open(map const & map, state_history & history, state_t state, int released_flow) -> int;
auto explore(map const & map, state_history & history, state_t state, int released_flow) -> int;

auto max(map const & map, state_history & history, state_t state = initial_state, int released_flow = 0) -> int
{
    // If our state matches a previous call to max, we already know the answer!
    auto found = history.find(state);

    if(found != history.end())
    {
        //std::cout << "Found previously reached state (from " << state.location << " with " << state.remaining_time << " minutes remaining), which is known to have a best additional release of " << found->second << std::endl;
        return released_flow + found->second;
    }

    if(state.remaining_time > 20)
        std::cout << "Finding max from " << state.location << " with " << state.remaining_time << " minutes remaining. We've already released " << released_flow << std::endl;
    
    // Model opening this valve
    auto best = open(map, history, state, released_flow);

    // Also model the exploration from this node without opening the valve
    best = std::max(best, explore(map, history, state, released_flow));

    // Remember this answer, might come in handy
    auto released = best - released_flow;
    if(released < 0)
    {
        std::cout << "We just released negative pressure: from " << state.location << " with " << state.remaining_time << " minutes remaining." << std::endl;
        throw 5;
    }
    history.insert(std::make_pair(state, best - released_flow));

    return best;
}

auto explore(map const & map, state_history & history, state_t current_state, int released_flow) -> int
{
    // Try each path from current location
    auto & next = map.at(current_state.location).next;

    // Use accumulate, but actually just for max
    return std::accumulate(next.begin(), next.end(), released_flow, [map, &history, released_flow, current_state](int best, auto destination)
    {
        auto state = current_state;
        // Walk to this destination, if possible
        if(state.remaining_time > destination.first)
        {
            state.location = destination.second;
            state.reduce_time(destination.first);
            
            auto mine = max(map, history, state, released_flow);
            //std::cout << "Returning max of " << best << "," << mine << std::endl;
            return std::max(best, mine);
        }
        //std::cout << "Returning " << best << std::endl;
        return best;
    });
}

auto open(map const & map, state_history & history, state_t state, int released_flow) -> int
{
    if(state.remaining_time <= 1)
    {
        // No point opening this valve
        return released_flow;
    }

    auto rate = map.at(state.location).rate;

    if(!state.opened.test(state.location))
    {
        state.opened.set(state.location);
        auto extra = (state.remaining_time - 1) * rate;
        state.reduce_time(1);
        //std::cout << "Opened valve in " << state.location << " with " << state.remaining_time << " minutes remeaining, which releases " << extra << " extra flow" << std::endl;
        released_flow += extra;
        //std::cout << released_flow << std::endl;
        auto ret = max(map, history, state, released_flow);
        //std::cout << ret << std::endl;
        return ret;
    }
    else
        return released_flow;
}

auto part1(map const & map) -> std::size_t
{
    state_history history;

    return max(map, history);//, 0, short_state);
}

auto part2(map const & map) -> std::size_t
{
    state_history history;

    return max(map, history, with_elephant);
}

int main()
{
    auto valves = parse();

    auto reduced = reduce(valves);

    std::cout << part1(reduced) << std::endl;

    //std::cout << part2(reduced) << std::endl;

    return 0;
}