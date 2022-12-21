#include <iostream>
#include <string>
#include <vector>

struct element_t
{
    element_t(int64_t v, std::size_t p, std::size_t n)
    : value(v), prev(p), next(n)
    {}
    int64_t value;
    std::size_t prev;
    std::size_t next;
};

using elements_t = std::vector<element_t>;


void print(elements_t const & values)
{
    std::size_t index = 0;

    do
    {
        auto & current = values.at(index);
        index = current.next;
        std::cout << current.value << ", ";
    }
    while(index != 0);

    std::cout << std::endl;
}

auto parse() -> elements_t
{
    std::string line;
    elements_t values;

    while(std::getline(std::cin, line))
    {
        auto size = values.size();
        values.emplace_back(stoi(line), size - 1, size + 1);
    }

    values.front().prev = values.size()-1;
    values.back().next = 0;

    return values;
}

void mix(elements_t & values)
{
    auto modulo = values.size() - 1;

    for(std::size_t i = 0; i < values.size(); ++i)
    {
        auto & current = values.at(i);

        //std::cout << "Removing value " << current.value << " from between " << values.at(current.prev).value << " and " << values.at(current.next).value << std::endl;
        
        // The current value is effectively being removed
        values.at(current.prev).next = current.next;
        values.at(current.next).prev = current.prev;

        // Now we need to find the destination, by walking backwards or forwards in the list
        auto dest = i;

        if(current.value >= 0)
        {
            // Walk forwards
            auto newdist = current.value % modulo;
            for(int64_t j = 0; j < newdist + 1; ++j)
                dest = values.at(dest).next;
        }
        else
        {
            // Walk backwards
            auto newdist = (0-current.value) % modulo;
            for(int64_t j = 0; j < newdist; ++j)
                dest = values.at(dest).prev;
        }

        // Link the current value in at the destination
        auto & next = values.at(dest);
        auto & prev = values.at(next.prev);

        current.next = dest;
        current.prev = next.prev;

        next.prev = i;
        prev.next = i;

        //print(values);

        //std::cout << "Inserting between " << prev.value << " and " << next.value << std::endl;
    }
}

auto coordinates(elements_t const & values) -> int64_t
{
    // Linear search, find the index of the starting value zero
    std::size_t start = 0;
    while(values.at(start).value != 0)
    {
        ++start;
    }

    std::cout << "Found " << values.at(start).value << " at " << start << std::endl;

    int64_t res = 0;

    for(int64_t i = 1000; i <= 3000; i += 1000)
    {
        auto offset = i % values.size();

        // Walk forwards enough
        auto idx = start;

        for(int64_t j = 0; j < offset; ++j)
        {
            idx = values.at(idx).next;
        }

        std::cout << "Value at position " << i << " after 0 is " << values.at(idx).value << std::endl;

        res += values.at(idx).value;
    }

    return res;
}


int main()
{
    auto orig = parse();

    auto values = orig;

    std::cout << "Mixing" << std::endl;
    mix(values);
    //print(values);

    std::cout << coordinates(values) << std::endl;

    auto part2 = orig;

    std::for_each(part2.begin(), part2.end(), [](auto & entry)
    {
        entry.value *= 811589153;
    });


    //print(part2);
    for(int i = 0; i < 10; ++i)
    {
        std::cout << "Mixing" << std::endl;
        mix(part2);
    }
    //print(part2);

    std::cout << coordinates(part2) << std::endl;


    return 0;
}