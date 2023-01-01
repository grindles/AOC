#include <iostream>
#include <list>
#include <optional>
#include <regex>
#include <set>
#include <string>

using pair = std::pair<std::string, std::string>;

auto parse() -> std::list<pair>
{
    std::string first, second;
    std::list<pair> ret;
    while(std::getline(std::cin, first))
    {
        std::getline(std::cin, second);
        ret.push_back(std::make_pair(first, second));
        if(std::getline(std::cin, second) && second.size() > 0)
        {
            std::cout << "Expected blank: " << second << std::endl;
            throw 0;
        }
    }
    return ret;
}

class packet_t
{
public:
    explicit packet_t(std::string & in);
    auto operator<(packet_t const & rhs) const -> bool;
    auto print() const -> std::string;
private:
    std::optional<int> m_value;
    std::list<packet_t> m_list;
};

// Construct a packet from a string. This consumes the string in parsed chunks
packet_t::packet_t(std::string & in)
: m_value{std::nullopt}, m_list{}
{
    std::regex regex("^(-?[0-9]+)");
    std::smatch match;

    if(in.front() == '[')
    {
        //std::cout << this << " Creating list based packet from " << in << std::endl;
        in.erase(in.begin());
        while(in.front() != ']')
        {
            //std::cout << this << " Creating list entry from " << in << std::endl;
            m_list.emplace_back(in);

            if(in.front() == ',')
                in.erase(in.begin());
        }
        in.erase(in.begin());
        //std::cout << this << " has completed list, remaining string is " << in << std::endl;
    }
    else if(std::regex_search(in, match, regex))
    {
        //std::cout << this << " Creating value based packet from " << in << std::endl;
        // Expected to be an integer
        m_value = stoi(match[1]);
        std::string whole = match[0];
        in.erase(0, whole.size()); // Next entry in the list is now ready
    }
    else
    {
        std::cout << "Failed to parse " << in << std::endl;
        throw 0;
    }

}

auto packet_t::print() const -> std::string
{
    
    if(m_value)
        return std::to_string(m_value.value());
    
    std::string ret;

    std::for_each(m_list.begin(), m_list.end(), [&ret](auto & pkt)
    {
        if(!ret.empty())
            ret.push_back(',');
        ret.append(pkt.print());
    });

    ret.insert(ret.begin(), '[');
    ret.append("]");

    return ret;
}

auto packet_t::operator<(packet_t const & rhs) const -> bool
{
    //std::cout << "Comparing " << print() << " with " << rhs.print() << std::endl;
    //If both values are integers, the lower integer should come first. If the left integer is lower than the right integer, the inputs are in the right order. If the left integer is higher than the right integer, the inputs are not in the right order. Otherwise, the inputs are the same integer; continue checking the next part of the input.
    if(m_value && rhs.m_value)
    {
        return m_value.value() < rhs.m_value.value();
    }
    //If both values are lists, compare the first value of each list, then the second value, and so on. If the left list runs out of items first, the inputs are in the right order. If the right list runs out of items first, the inputs are not in the right order. If the lists are the same length and no comparison makes a decision about the order, continue checking the next part of the input.
    else if(!m_value && !rhs.m_value)
    {
        return m_list < rhs.m_list;
    }
    //If exactly one value is an integer, convert the integer to a list which contains that integer as its only value, then retry the comparison. For example, if comparing [0,0,0] and 2, convert the right value to [2] (a list containing 2); the result is then found by instead comparing [0,0,0] and [2].
    else if(m_value)
    {
        // RHS is a list, so make a list to match
        std::list<packet_t> me_as_list{*this};
        return me_as_list < rhs.m_list;
    }
    else
    {
        // LHS is a list
        std::list<packet_t> them_as_list{rhs};
        return m_list < them_as_list;
    }
}

auto part1(std::list<pair> pairs) -> std::size_t
{
    std::size_t index = 0;
    std::size_t score = 0;

    std::for_each(pairs.begin(), pairs.end(), [&index, &score](auto current)
    {
        ++index;
        //std::cout << "Comparing " << current.first << " vs " << current.second << ":";
        packet_t lhs{current.first};

        packet_t rhs{current.second};

        if(lhs < rhs)
        {
            //std::cout << " less than!";
            score += index;
        }

        //std::cout << std::endl;

    });

    return score;
}

auto index_of(std::set<packet_t> const & packets, packet_t search) -> std::size_t
{
    auto iter = packets.find(search);

    if(iter == packets.end())
    {
        throw 5;
    }

    auto distance = std::distance(packets.begin(), iter);

    return 1 + distance;
}

auto part2(std::list<pair> pairs)
{
    std::set<packet_t> packets;

    std::string div1_str{"[[2]]"};
    std::string div2_str{"[[6]]"};

    packet_t div1{div1_str};
    packet_t div2{div2_str};

    packets.insert(div1);
    packets.insert(div2);

    std::for_each(pairs.begin(), pairs.end(), [&packets](auto current)
    {
        packets.insert(packet_t{current.first});
        packets.insert(packet_t{current.second});
    });

    return index_of(packets, div1) * index_of(packets, div2);
}

int main()
{
    auto pairs = parse();

    std::cout << part1(pairs) << std::endl;
    std::cout << part2(pairs) << std::endl;

    return 0;
}