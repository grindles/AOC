#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using grid = std::vector<std::string>;

auto readgrid() -> grid
{
    grid trees;
    std::string line;

    while(std::getline(std::cin, line))
    {
        trees.push_back(line);
    }
    return trees;
}

auto visible(grid const & trees) -> std::size_t
{
    std::size_t total = 0;

    for(auto yit = trees.begin(); yit != trees.end(); ++yit)
    {
        auto & row = *yit;
        for(auto xit = row.begin(); xit != row.end(); ++xit)
        {
            auto xoff = std::distance(row.begin(), xit);
            auto check = *xit;
            if(std::none_of(row.begin(), xit, [check](auto tree)
            {
                return tree >= check;
            }) ||
            std::none_of(xit + 1, row.end(), [check](auto tree)
            {
                return tree >= check;
            }) ||
            std::none_of(trees.begin(), yit, [xoff, check](auto treerow)
            {
                return treerow.at(xoff) >= check;
            }) || 
            std::none_of(yit+1, trees.end(), [xoff, check](auto treerow)
            {
                return treerow.at(xoff) >= check;
            }))
            {
                ++total;
            }
        }
    }
    return total;
}

auto scenic(grid const & trees) -> std::size_t
{
    std::size_t best = 0;
    std::size_t height = trees.size();
    std::size_t width = trees.front().size();

    for(std::size_t y = 0; y < height; ++y)
    {
        auto row = trees.at(y);
        for(std::size_t x = 0; x < width; ++x)
        {
            auto tree = trees.at(y).at(x);
            std::size_t left = 0;
            std::size_t right = 0;
            std::size_t up = 0;
            std::size_t down = 0;

            for(int64_t i = x-1; i >= 0; --i)
            {
                ++left;
                if(row.at(i) >= tree)
                    break;
            }

            for(int64_t i = x+1; i < width; ++i)
            {
                ++right;
                if(row.at(i) >= tree)
                    break;
            }

            for(int64_t i = y-1; i >= 0; --i)
            {
                ++up;
                if(trees.at(i).at(x) >= tree)
                    break;
            }

            for(int64_t i = y+1; i < height; ++i)
            {
                ++down;
                if(trees.at(i).at(x) >= tree)
                    break;
            }

            auto score = left * right * up * down;
            if(score > best)
            {
                best = score;
            }
        }
    }

    return best;
}

int main()
{
    auto trees = readgrid();

    std::cout << visible(trees) << std::endl;
    std::cout << scenic(trees) << std::endl;
}