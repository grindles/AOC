#include <bitset>
#include <iostream>
#include <string>
#include <vector>

static constexpr std::size_t MAX = 53;

int charval(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - 'a' + 1;
    }
    else if(c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 27;
    }
    throw 5;
}

int main()
{
    std::string line;
    std::size_t total = 0;

    using itemset = std::bitset<MAX>;

    std::vector<itemset> elves;

    while(getline(std::cin, line))
    {
        auto length = line.size();
        auto half   = length / 2;

        itemset combined, left, right, intersection;

        for(int i = 0; i < half; i++)
        {
            auto ch = line.at(i);
            auto val = charval(ch);
            left.set(val);

            ch = line.at(half + i);
            val = charval(ch);
            right.set(val);
        }

        intersection = left & right;
        combined = left | right;

        for(int i = 0; i < MAX; ++i)
        {
            if(intersection[i])
            {
                total += i;
            }
        }
        
        elves.push_back(combined);
    }

    std::cout << total << std::endl;

    total = 0;

    auto count = elves.size();

    for(int i = 0; i < count; i += 3)
    {
        auto shared = elves.at(i) & elves.at(i+1) & elves.at(i+2);

        if(shared.count() != 1)
        {
            throw 6;
        }

        for(int i = 0; i < MAX; ++i)
        {
            if(shared[i])
            {
                total += i;
            }
        }
    }

    std::cout << total << std::endl;

    return 0;
}