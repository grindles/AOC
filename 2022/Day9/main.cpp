#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <vector>

using pos_t = std::pair<int, int>;

auto move_tail(pos_t head, pos_t tail) -> pos_t
{
    auto y_dist = std::abs(head.first - tail.first);
    auto x_dist = std::abs(head.second - tail.second);

    if(y_dist > 1 || x_dist > 1)
    {
        if(tail.first > head.first)
            --tail.first;
        if(tail.first < head.first)
            ++tail.first;
        if(tail.second > head.second)
            --tail.second;
        if(tail.second < head.second)
            ++tail.second;
    }
    return tail;
}

std::size_t length = 10;

int main()
{
    std::string line;
    std::regex rgx{"^([UDLR]) ([0-9]+)$"};
    std::smatch matches;

    std::vector<pos_t> knots{length, {0,0}};

    pos_t & head = knots.front();
    pos_t & tail = knots.back();

    std::set<pos_t> visited;
    visited.insert(tail);

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, matches, rgx))
        {
            throw 5;
        }

        std::string dir = matches[1];
        auto count = stoi(matches[2]);

        for(std::size_t i = 0; i < count; ++i)
        {
            switch(dir[0])
            {
                case 'R':
                    ++head.second;
                    break;
                case 'L':
                    --head.second;
                    break;
                case 'U':
                    ++head.first;
                    break;
                case 'D':
                    --head.first;
                    break;
                default:
                    throw 5;
            }

            for(std::size_t k = 1; k < length; ++k)
            {
                knots.at(k) = move_tail(knots.at(k-1), knots.at(k));
            }

            visited.insert(tail);
        }
    }

    std::cout << visited.size() << std::endl;

}