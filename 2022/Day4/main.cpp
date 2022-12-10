#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

bool containing(int A, int B, int C, int D)
{
    if(A > B) std::swap(A, B);
    if(C > D) std::swap(C, D);

    if(A >= C && B <= D) return true;
    if(C >= A && D <= B) return true;

    return false;
}

bool overlapping(int A, int B, int C, int D)
{
    if(A > B) std::swap(A, B);
    if(C > D) std::swap(C, D);

    if(A <= D && B >= C) return true;
    if(D <= A && C >= B) return true;

    return false;
}

int main()
{
    std::regex assignregex{"^([0-9]+)-([0-9]+),([0-9]+)-([0-9]+)$"};
    std::smatch match;
    std::string line;

    std::size_t contain = 0;
    std::size_t overlap = 0;

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, match, assignregex))
        {
            throw 5;
        }
        std::array<int, 4> values;
        for(int i = 0; i < 4; ++i)
        {
            values[i] = stoi(match[i+1]);
        }

        if(containing(values[0], values[1], values[2], values[3]))
        {
            ++contain;
        }
        if(overlapping(values[0], values[1], values[2], values[3]))
        {
            ++overlap;
        }
    }

    std::cout << contain << std::endl;
    std::cout << overlap << std::endl;
    return 0;
}