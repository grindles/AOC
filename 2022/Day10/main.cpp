#include <iostream>
#include <regex>
#include <vector>

enum opcode_t {noop, addx};

using instruction_t = std::pair<opcode_t, int>;
using program_t = std::vector<instruction_t>;

auto read_program() -> program_t
{
    program_t prog;
    std::string line;
    std::regex rgx{"(noop|addx) ?(-?[0-9]+)?"};
    std::smatch matches;

    while(std::getline(std::cin, line))
    {
        if(!std::regex_match(line, matches, rgx))
        {
            throw 5;
        }

        std::string opcode = matches[1];

        if(opcode == "noop")
        {
            prog.emplace_back(noop, 0);
        }
        else
        {
            auto value = stoi(matches[2]);
            prog.emplace_back(addx, value);
        }
    }

    return prog;
}

auto run_program(program_t const & prog) -> std::vector<int>
{
    std::vector<int> signal;
    int x = 1;
    std::size_t cycle = 0;

    signal.push_back(x * ++cycle);

    std::for_each(prog.begin(), prog.end(), [&x, &cycle, &signal](auto inst)
    {
        switch(inst.first)
        {
            case noop:
                signal.push_back(x);
                break;
            case addx:
                // Two cycles to add
                signal.push_back(x);
                signal.push_back(x);
                x += inst.second;
                break;
            default:
                throw 5;
        }

    });

    return signal;
}

int main()
{
    auto prog = read_program();

    auto sig = run_program(prog);

    // part 1
    std::size_t signal = 0;

    signal += sig.at(20) * 20;
    signal += sig.at(60) * 60;
    signal += sig.at(100) * 100;
    signal += sig.at(140) * 140;
    signal += sig.at(180) * 180;
    signal += sig.at(220) * 220;

    std::cout << signal << std::endl;

    // part 2
    for(int row = 0; row < 6; ++row)
    {
        std::string line;
        for(int i = 0; i < 40; ++i)
        {
            auto x = sig.at(row*40+i+1);

            if(std::abs(x-i) < 2)
            {
                line.push_back('#');
            }
            else
            {
                line.push_back('.');
            }
        }

        std::cout << line << std::endl;
    }

    return 0;
}