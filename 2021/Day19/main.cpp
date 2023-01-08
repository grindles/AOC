#include <algorithm>
#include <array>
#include <limits>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <vector>

static constexpr std::size_t dims = 3;
using pos_t = std::array<int, dims>;
using pos_set_t = std::set<pos_t>;

static constexpr pos_t zero{0,0,0};

static constexpr std::size_t overlapping_beacons = 12;
static constexpr std::size_t overlapping_distances = (overlapping_beacons * (overlapping_beacons-1)) / 2;

void print(pos_t a)
{
    std::cout << a[0] << "," << a[1] << "," << a[2];
}

auto operator-(pos_t const & a, pos_t const & b) -> pos_t
{
    pos_t ret{a};
    for(int i = 0; i < dims; ++i)
        ret[i] -= b[i];
    return ret;
}

auto operator+(pos_t const & a, pos_t const & b) -> pos_t
{
    pos_t ret{a};
    for(int i = 0; i < dims; ++i)
        ret[i] += b[i];
    return ret;
}

auto distance(pos_t const & a, pos_t const & b) -> std::size_t
{
    //std::cout << "Distance between " << print(a) << " and " << print(b) << std::endl;
    auto diff = a - b;
    std::size_t ret = 0;
    for(int i = 0; i < dims; ++i)
    {
        ret += std::abs(diff[i]);
    }
    return ret;
}

// Rotate a position vector one of 24 different ways
// Model cube looking down from top.
// x is left to right
// y is bottom to top
// z is down to up
auto rotate(pos_t a, int face, int rotate) -> pos_t
{
    auto which_face = face % 6;
    // Rotate which face is towards us
    switch(which_face)
    {
        case 1: // Take up and point it towards us; y -> z, +z -> -y
            std::swap(a[1], a[2]);
            a[1] *= -1;
            break; 
        case 2: // Take right and point it towards us; x -> z, +z -> -x
            std::swap(a[0], a[2]);
            a[0] *= -1;
            break;
        case 3: // Take down and point it towards us; -y -> +z, z -> y
            std::swap(a[1], a[2]);
            a[2] *= -1;
            break;
        case 4: // Take left and point it towards us; -x -> +z, z -> x
            std::swap(a[0], a[2]);
            a[2] *= -1;
            break;
        case 5: // Take back and point it towards us, rotating along x axix; -y -> +y, -z -> +z
            a[1] *= -1;
            a[2] *= -1;
            break;
        default: // Nothing to do here
            break;
    }

    auto which_rotate = rotate % 4;
  
    // Now rotate the face that points towards us, clockwise along the z axis
    for(int i = 0; i < which_rotate; ++i)
    {
        // +y -> -x, x -> y
        std::swap(a[0], a[1]);
        a[0] *= -1;
    }
    return a;
}

class scanner_t
{
public:
    explicit scanner_t(pos_set_t const & pos);
    // Try and combine with another scanner
    auto combine(scanner_t const & other) -> bool;

    auto size() const -> std::size_t;
    auto range() const -> std::size_t;
private:
    scanner_t() = default;
    auto rot(int face, int rot) const -> scanner_t;

    auto shiftcombine(scanner_t const & rotated, std::map<pos_t, std::pair<pos_t, pos_t>> const & intersect) -> bool;

    //using index_pair = std::pair<std::size_t, std::size_t>;
    //using difference_set = std::set<pos_t>;
    using difference_mapping = std::map<pos_t, pos_t>;
    //auto differences(int rot) -> difference_set;
    // Get the differences between each pair
    auto differences() const -> difference_mapping;
    // Get the intersection of differences, mapping them to mine and theirs
    static auto intersection(difference_mapping const & mine, difference_mapping const & theirs) -> std::map<pos_t, std::pair<pos_t, pos_t>>;
    // Get the manhattan distances between each pair
    //auto distances() const -> std::set<int>;
    
    pos_set_t m_beacons;
    pos_set_t m_scanners;
};

scanner_t::scanner_t(pos_set_t const & pos)
: m_beacons{pos}
{}

auto scanner_t::size() const -> std::size_t
{
    return m_beacons.size();
}

auto scanner_t::range() const -> std::size_t
{
    std::size_t ret = 0;

    std::for_each(m_scanners.begin(), m_scanners.end(), [this, &ret](auto first)
    {
        std::for_each(m_scanners.begin(), m_scanners.end(), [first, &ret](auto second)
        {
            auto dist = distance(first, second);
            std::cout << "Measured distance of " << dist << " from ";
            print(first);
            std::cout << " to ";
            print(second);
            std::cout << std::endl;
            if(dist > ret)
            {
                ret = dist;
            }
        });
    });

    return ret;
}

auto scanner_t::rot(int face, int rot) const -> scanner_t
{
    scanner_t ret{};

    std::for_each(m_beacons.begin(), m_beacons.end(), [face, rot, &ret](auto beacon)
    {
        ret.m_beacons.insert(rotate(beacon, face, rot));
    });

    std::for_each(m_scanners.begin(), m_scanners.end(), [face, rot, &ret](auto scanner)
    {
        ret.m_scanners.insert(rotate(scanner, face, rot));
    });

    return ret;
}

auto scanner_t::shiftcombine(scanner_t const & rotated, std::map<pos_t, std::pair<pos_t, pos_t>> const & intersect) -> bool
{

    // A = B - C
    // C = B - A
    auto first = intersect.begin();
    if(first == intersect.end())
    {
        std::cout << "Intersection size is zero\n";
        throw 5;
    }

    auto shift = first->second.first - first->second.second;

    std::for_each(rotated.m_beacons.begin(), rotated.m_beacons.end(), [this, shift](auto pos)
    {
        auto shifted = pos + shift;
        m_beacons.insert(shifted);
    });

    // Remember where the scanner was
    m_scanners.insert(zero + shift);
    
    // And any other scanners it knew about
    std::for_each(rotated.m_scanners.begin(), rotated.m_scanners.end(), [this, shift](auto pos)
    {
        auto shifted = pos + shift;
        m_scanners.insert(shifted);
    });

    return true;
}

auto scanner_t::combine(scanner_t const & other) -> bool
{
    std::cout << "Combining scanners\n";
    for(int f = 0; f < 6; ++f)
    {
        for(int r = 0; r < 4; ++r)
        {
            auto rotated = other.rot(f, r);
            std::cout << "Trying face " << f << ", rotation " << r << "\n";

            auto mine = differences();
            auto theirs = rotated.differences();

            auto intersect = intersection(mine, theirs);

            if(intersect.size() >= overlapping_distances)
            {
                std::cout << "Found " << intersect.size() << " intersections\n";
                shiftcombine(rotated, intersect);
                std::cout << "Combination complete, now have " << size() << "\n";
                return true;
            }
        }
    }
    return false;
}

auto scanner_t::intersection(difference_mapping const & mine, difference_mapping const & theirs) -> std::map<pos_t, std::pair<pos_t, pos_t>>
{
    std::map<pos_t, std::pair<pos_t, pos_t>> ret;

    std::for_each(mine.begin(), mine.end(), [theirs, &ret](auto lhs)
    {
        auto found = theirs.find(lhs.first);
        if(found != theirs.end())
        {
            ret[lhs.first] = std::make_pair(lhs.second, found->second);
        }
    });

    return ret;
}

/*auto scanner_t::differences(int rot) -> difference_set
{
    auto rotated = m_beacons;
    std::for_each(rotated.begin(), rotated.end(), [](auto & pos){pos = rotate(pos, rot)});

    for(std::size_t first = 0; first < size(); ++first)
    {
        for(std::size_t second = 0; second < size(); ++second)
        {
            if(first != second)
            {
                auto diff = m_beacons[first] - m_beacons[second];
                ret[diff] = std::make_pair(first, second);
            }
        }
    }
}*/

auto scanner_t::differences() const -> difference_mapping
{
    difference_mapping ret;

    std::for_each(m_beacons.begin(), m_beacons.end(), [this, &ret](auto first)
    {
        std::for_each(m_beacons.begin(), m_beacons.end(), [first, &ret](auto second)
        {
            if(first != second)
            {
                auto diff = first - second;
                ret[diff] = first;
            }
        });
    });

    return ret;
}

/*auto scanner_t::distances() const -> std::set<int>
{
    std::set<int> ret;

    std::for_each(m_beacons.begin(), m_beacons.end(), [this, &ret](auto & first)
    {
        std::for_each(m_beacons.begin(), m_beacons.end(), [&ret, first](auto & second)
        {
            if(first != second)
            {
                ret.insert(distance(first, second));
            }
        });
    });

    return ret;
}*/

auto get_scanner() -> std::pair<int, scanner_t>
{
    std::string line;
    std::regex namergx{"^--- scanner ([0-9]+) ---$"};
    std::regex posrgx{"^(-?[0-9]+),(-?[0-9]+),(-?[0-9]+)$"};
    std::smatch matches;

    std::getline(std::cin, line);
    if(!std::regex_match(line, matches, namergx))
    {
        //std::cout << "Failed to parse scanner name " << line << std::endl;
        throw 1;
    }

    int id = stoi(matches[1].str());

    pos_set_t positions;

    while(std::getline(std::cin, line) && std::regex_match(line, matches, posrgx))
    {
        pos_t pos;
        for(int i = 0; i < dims; ++i)
        {
            pos.at(i) = stoi(matches[i+1].str());
        }
        positions.insert(pos);
    }

    return std::make_pair(id, scanner_t{positions});
}

auto get_scanners() -> std::map<int, scanner_t>
{
    std::map<int, scanner_t> scanners;

    while(true)
    {
        try
        {
            scanners.insert(get_scanner());
        }
        catch(...)
        {
            return scanners;
        }
    }
    return scanners;
}

// Combine all the scanners together
auto combine(std::map<int, scanner_t> scanners) -> scanner_t
{
    while(scanners.size() > 1)
    {
        bool have_combined = false;
        for(auto dest = scanners.begin(); dest != scanners.end(); ++dest)
        {
            auto next = dest;
            std::advance(next, 1);
            for(auto source = next; source != scanners.end(); ++source)
            {
                if(dest->second.combine(source->second))
                {
                    std::cout << "Erasing\n";
                    scanners.erase(source->first);
                    next = dest;
                    have_combined = true;
                    break;
                }
            }
        }

        if(!have_combined)
        {
            std::cout << "Didn't find any scanners to combine, still have " << scanners.size() << " remaining" << std::endl;
            throw 5;
        }
    }

    return scanners.begin()->second;
}

int main()
{
    auto scanners = get_scanners();

    auto combined = combine(scanners);

    std::cout << combined.size() << std::endl;
    std::cout << combined.range() << std::endl;

    return 0;
}