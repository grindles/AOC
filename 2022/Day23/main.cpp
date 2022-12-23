#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

using elf_t = std::pair<int64_t, int64_t>;
using elves_t = std::set<elf_t>;
using neighbours_t = std::bitset<8>;

auto parse() -> elves_t
{
    elves_t ret;
    std::string line;
    int64_t row = 0;

    while(std::getline(std::cin, line))
    {
        for(int64_t col = 0; col < line.size(); ++col)
            if(line[col] == '#')
                ret.insert(elf_t{row, col});
        ++row;
    }
    return ret;
}

// Checks for neighbours in a direction, returning the proposed destination or 
auto neighbours(elves_t const & elves, elf_t current, int dir) -> std::optional<elf_t>
{
    auto pos = current;
    bool row = false;
    switch(dir)
    {
        case 0: pos.first--; row = false; break; // North
        case 1: pos.first++; row = false; break; // South
        case 2: pos.second--; row = true; break; // West
        case 3: pos.second++; row = true; break; // East
        default: throw 5;
    }

    //std::cout << "Checking around " << pos.first << "," << pos.second << std::endl;

    bool clash = false;

    if(row)
    {
        // Walk the rows
        for(elf_t check{pos.first-1, pos.second}; check.first <= pos.first+1; ++check.first)
        {
            //std::cout << "Checking at " << check.first << "," << check.second << std::endl;
            clash = clash || elves.contains(check);
            //std::cout << clash << std::endl;
        }
    }
    else
    {
        // Walk the columns
        for(elf_t check{pos.first, pos.second-1}; check.second <= pos.second+1; ++check.second)
        {
            //std::cout << "Checking at " << check.first << "," << check.second << std::endl;
            clash = clash || elves.contains(check);
            //std::cout << clash << std::endl;
        }
    }
    if(!clash)
    {
        //std::cout << "Can move " << dir << std::endl;
        return pos;
    }
    else
        return std::nullopt;
}

static std::list<int> dirpirs = {0,1,2,3};

auto propose(elves_t const & elves, elf_t current) -> elf_t
{
    bool none = true;
    auto prop = current;

    std::for_each(dirpirs.rbegin(), dirpirs.rend(), [elves, current, &none, &prop](auto dir)
    {
        auto res = neighbours(elves, current, dir);
        none = none && res;
        if(res)
            prop = res.value();
    });

    if(none)
        return current;
    return prop;
}

using proposals_t = std::multimap<elf_t, elf_t>;

auto proposals(elves_t const & elves) -> proposals_t
{
    proposals_t ret;

    std::for_each(elves.begin(), elves.end(), [&ret, elves](auto elf)
    {
        auto dst = propose(elves, elf);
        //std::cout << "Elf at " << elf.first << "," << elf.second << " proposing " << dst.first << "," << dst.second << std::endl;
        ret.insert(std::make_pair(dst, elf));
    });

    // Rotate direction priorities

    dirpirs.push_back(dirpirs.front());
    dirpirs.pop_front();

    return ret;
}

auto resolve(proposals_t const & props) -> elves_t
{
    elves_t ret;

    std::for_each(props.begin(), props.end(), [&ret, props](auto current)
    {
        elf_t dest = current.first;

        // Check if this proposed destination is unique
        if(props.count(current.first) > 1)
        {
            //std::cout << "Elf destination " << dest.first << "," << dest.second << " has a clash" << std::endl;
            dest = current.second;
        }

        //std::cout << "Inserting elf at " << dest.first << "," << dest.second << "(from " << current.second.first << "," << current.second.second << ")" << std::endl;
        auto chk = ret.insert(dest).second;

        if(!chk)
        {
            std::cout << "Failed to insert an elf" << std::endl;
            throw 5;
        }

    });

    return ret;
}

auto resolver(elves_t & elves, proposals_t const & props) -> std::size_t
{
    std::size_t ret = 0;
    std::for_each(props.begin(), props.end(), [&elves, &ret, props](auto current)
    {
        elf_t dest = current.first;

        // Check if this proposed destination is unique
        if(props.count(current.first) == 1 && current.first != current.second)
        {
            elves.erase(current.second);
            auto chk = elves.insert(current.first).second;

            if(!chk)
            {
                std::cout << "Failed to insert an elf" << std::endl;
                throw 5;
            }

            ++ret;
        }

    });

    return ret;
}

auto gridarea(elves_t const & elves) -> std::size_t
{
    elf_t max{std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min()};
    elf_t min{std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::max()};

    std::for_each(elves.begin(), elves.end(), [&min, &max](auto elf)
    {
        min.first = std::min(min.first, elf.first);
        max.first = std::max(max.first, elf.first);
        min.second = std::min(min.second, elf.second);
        max.second = std::max(max.second, elf.second);
    });

    return (max.first + 1 - min.first) * (max.second + 1 - min.second);
}

void print(elves_t const & elves)
{
    for(int64_t row = 0; row < 12; ++row)
    {
        std::string line;
        for(int64_t col = 0; col < 14; ++col)
        {
            if(elves.contains(elf_t{row, col}))
                line.push_back('#');
            else
                line.push_back('.');
        }

        std::cout << line << std::endl;
    }
}

auto part1(elves_t elves) -> std::size_t
{
    for(int round = 0; round < 10; ++round)
    {
        std::cout << "Round " << round << " with " << elves.size() << std::endl;
        print(elves);
        auto props = proposals(elves);
        //std::cout << "Proposals " << props.size() << std::endl;
        elves = resolve(props);

    }

    print(elves);

    return gridarea(elves) - elves.size();
    
}

auto part2(elves_t elves) -> std::size_t
{
    int round = 0;
    while(true)
    {
        ++round;
        std::cout << "Round " << round << " with " << elves.size() << std::endl;
        auto props = proposals(elves);
        //std::cout << "Proposals " << props.size() << std::endl;
        auto newelves = resolve(props);
        
        if(newelves == elves)
        {
            return round + 1;
        }

        elves = newelves;
    }

}
int main()
{
    auto elves = parse();
    std::cout << part1(elves) << std::endl;
    std::cout << part2(elves) << std::endl;
    return 0;
}