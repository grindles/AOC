#include <iostream>
#include <string>

int main()
{
    std::string line;
    
    while(std::getline(std::cin, line))
    {
        auto box = line.substr(0, 3);

        line = line.substr(3);
    }

} 