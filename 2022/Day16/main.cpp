#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string>

using name_t = std::string;
using neighbour_t = std::pair<int, name_t>;

struct node_t
{
    int rate;
    std::list<neighbour_t> next;
};

using map = std::map<name_t, node_t>;

auto destinations(std::string csl) -> std::list<neighbour_t>
{
    std::list<neighbour_t> ret;

    while(csl.length() >= 2)
    {
        ret.push_back(std::make_pair(1, csl.substr(0, 2)));
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

auto shortest(name_t from, name_t to, map const & map) -> int
{
    if(from == to)
        return 0;
    //std::cout << "Finding shortest path between " << from << " and " << to << ": ";
    auto from_iter = map.find(from);
    if(from_iter == map.end())
    {
        throw 5;
    }

    // BFS from source until matching destination is reached
    std::multimap<int, name_t> queue;
    auto & froms = from_iter->second.next;

    std::for_each(froms.begin(), froms.end(), [&queue](auto f)
    {
        //std::cout << "Initial queue includes " << f.second << std::endl;
        queue.insert(f);
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

        auto extra_iter = map.find(next);
        if(extra_iter == map.end())
        {
            throw 6;
        }
        auto extra_hops = extra_iter->second.next;

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

    auto node = graph.find(source);
    auto & neighbours = node->second.next;

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
            auto node = graph.find(name);

            if(node == graph.end())
            {
                throw 5;
            }

            auto & neighbours = node->second.next;

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
    std::set<name_t> interesting{"AA"}; // AA is always interesting as it's the start node

    // Filter the list of nodes to those with non-zero flow rate
    std::for_each(graph.begin(), graph.end(), [&interesting](auto entry)
    {
        if(entry.second.rate > 0)
        {
            interesting.insert(entry.first);
        }
    });

    std::cout << "Found " << interesting.size() << " nodes of interest out of " << graph.size() << std::endl;

    // For each interesting node, find it's shortest path to neighbours
    map reduced;

    // Create mapping between each pair of points
    std::for_each(interesting.begin(), interesting.end(), [interesting, graph, &reduced](auto source)
    {
        auto rate = graph.find(source)->second.rate;
        // New node
        auto iter = reduced.insert(std::make_pair(source, node_t{rate, std::list<neighbour_t>{}})).first;
        std::list<neighbour_t> & next = iter->second.next;

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
        return opened < rhs.opened;
    }

    int remaining_time;
    name_t location;
    std::set<name_t> opened;
};

static const state_t initial_state{30, "AA", std::set<name_t>{}};

using state_history = std::map<state_t, int>;

auto open(map const & map, state_history & history, int released_flow, state_t state) -> int;
auto explore(map const & map, state_history & history, int released_flow, state_t state) -> int;

auto max(map const & map, state_history & history, int released_flow = 0, state_t state = initial_state) -> int
{
    // If our state matches a previous call to max, we already know the answer!
    auto found = history.find(state);

    if(found != history.end())
    {
        //std::cout << "Found previously reached state (from " << state.location << " with " << state.remaining_time << " minutes remaining), which is known to have a best additional release of " << found->second << std::endl;
        return released_flow + found->second;
    }

    if(state.remaining_time > 27)
        std::cout << "Finding max from " << state.location << " with " << state.remaining_time << " minutes remaining. We've already released " << released_flow << std::endl;
    // Model opening this valve
    auto best = open(map, history, released_flow, state);

    // Also model the exploration from this node without opening the valve
    best = std::max(best, explore(map, history, released_flow, state));

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

auto explore(map const & map, state_history & history, int released_flow, state_t current_state) -> int
{
    // Try each path from current location
    auto found = map.find(current_state.location);
    auto & next = found->second.next;

    // Use accumulate, but actually just for max
    return std::accumulate(next.begin(), next.end(), released_flow, [map, &history, released_flow, current_state](int best, auto destination)
    {
        auto state = current_state;
        // Walk to this destination, if possible
        if(state.remaining_time > destination.first)
        {
            state.remaining_time -= destination.first;
            state.location = destination.second;
            auto mine = max(map, history, released_flow, state);
            //std::cout << "Returning max of " << best << "," << mine << std::endl;
            return std::max(best, mine);
        }
        //std::cout << "Returning " << best << std::endl;
        return best;
    });
}

auto open(map const & map, state_history & history, int released_flow, state_t state) -> int
{
    if(state.remaining_time <= 1)
    {
        // No point opening this valve
        return released_flow;
    }
    auto found = map.find(state.location);

    if(state.opened.insert(state.location).second)
    {
        --state.remaining_time;
        auto extra = state.remaining_time * found->second.rate;
        //std::cout << "Opened valve in " << state.location << " with " << state.remaining_time << " minutes remeaining, which releases " << extra << " extra flow" << std::endl;
        released_flow += extra;
        //std::cout << released_flow << std::endl;
        auto ret = explore(map, history, released_flow, state);
        //std::cout << ret << std::endl;
        return ret;
    }
    else
        return released_flow;
}

auto part1(map const & map) -> std::size_t
{
    auto reduced = reduce(map);

    state_history history;

    return max(reduced, history);//, 0, short_state);
}

int main()
{
    auto valves = parse();

    std::cout << part1(valves) << std::endl;

    return 0;
}