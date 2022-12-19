#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

//using shape_t = uint64_t;
//using chamber_t = uint64_t;

using shape_t = std::vector<uint8_t>;
using chamber_t = std::vector<uint8_t>;
using shapes_t = std::vector<shape_t>;

shapes_t shapes = {{0x1e}, {0x08, 0x1c, 0x08}, {0x04, 0x04, 0x1c}, {0x10, 0x10, 0x10, 0x10}, {0x18, 0x18}};

/*constexpr std::array<shape_t, 5> shapes{0x0000001e00000000,
                                        0x0000081c00000000,
                                        0x0004041c00000000,
                                        0x1010101000000000,
                                        0x0000181800000000};*/

struct wind_t
{
    explicit wind_t(const std::string in)
    : pattern{in}, index{0}
    {}

    auto next_left() -> bool
    {
        auto ch = pattern.at(index);
        ++index;

        if(index == pattern.size())
            index = 0;
        return ch == '<';
    }

    std::string pattern;
    std::size_t index;
};

auto parse_wind() -> wind_t
{
    std::string line;
    std::getline(std::cin, line);
    return wind_t{line};
}

auto shift(shape_t const & shape, chamber_t const & chamber, bool left) -> shape_t
{
    auto shifted = shape;
    for(std::size_t i = 0; i < shape.size(); ++i)
    {
        if(left)
        {
            shifted.at(i) = shape.at(i) << 1;
            if(shape.at(i) & 0xc0)
                return shape;
        }
        else
        {
            shifted.at(i) = shape.at(i) >> 1;
            if(shape.at(i) & 0x01)
                return shape;
        }
        if(shifted.at(i) & chamber.at(i))
            return shape;
    }

    return shifted;
}

auto check_hit(shape_t const & shape, chamber_t const & chamber) -> bool
{
    auto limit = std::min(shape.size(), chamber.size());

    for(size_t i = 0; i < limit; ++i)
    {
        if(shape.at(i) & chamber.at(i))
            return true;
    }
    return false;
}

auto print(std::vector<uint8_t> const & v) -> std::string
{
    std::ostringstream o;
    for(size_t i = 0; i < v.size(); ++i)
    {
        o << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(v.at(i));
    }
    return o.str();
}

void add_shape(shape_t const & shape, chamber_t & chamber)
{
    //std::cout << "Adding " << print(shape) << " to " << print(chamber) << std::endl;
    auto limit = std::min(shape.size(), chamber.size());

    for(size_t i = 0; i < limit; ++i)
    {
        chamber.at(i) |= shape.at(i);
    }

    // Remove any leading zeros
    while(chamber.front() == 0x00)
    {
        chamber.erase(chamber.begin());
    }
}

void process_shape(wind_t & wind, chamber_t & chamber, shape_t shape)
{
    // Pad chamber to be 3 empty rows under shape
    chamber.insert(chamber.begin(), shape.size() + 3, 0x00);

    //std::cout << "Adding shape of length " << shape.size() << " to chamber " << chamber.size() << std::endl;

    while(true)
    {
        // Shift based on wind, if possible
        auto left = wind.next_left();

        auto shifted = shift(shape, chamber, left);

        //std::cout << "Blown shape now " << print(shifted) << std::endl;

        auto dropped = shifted;

        // Move shape down
        dropped.insert(dropped.begin(), 1, 0x00);

        //std::cout << "Dropped shape now " << print(dropped) << std::endl;

        if(dropped.size() > chamber.size())
        {
            throw 5;
        }

        if(check_hit(dropped, chamber))
        {
            // Merge shape into chamber
            add_shape(shifted, chamber);
            // We're done
            return;
        }

        shape = dropped;
    }
}

auto chamber_state(chamber_t const & chamber) -> std::pair<chamber_t, std::size_t>
{
    auto iter = chamber.begin();
    chamber_t ret;
    uint8_t check = 0x7f;

    while(check && iter != chamber.end())
    {
        auto ch = *iter;

        ret.push_back(ch);
        ++iter;
        check = check & ~ch;
    }

    //return std::make_pairret
}

int main()
{
    auto wind = parse_wind();
    chamber_t chamber{0x7f};

    for(int i = 0; i < 2022; ++i)
    {
        process_shape(wind, chamber, shapes.at(i % shapes.size()));
        //std::cout << print(chamber) << std::endl;
    }

    std::cout << chamber.size() - 1 << std::endl;

    // Part 2
    return 0;
}