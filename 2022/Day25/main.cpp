#include <iostream>
#include <string>

auto digit_to_int(char ch) -> int
{
    switch(ch)
    {
        case '=': return -2;
        case '-': return -1;
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        default:
            std::cout << "Bad digit " << ch << std::endl;
            throw 5;
    }
}

auto int_to_digit(int x) -> char
{
    switch(x)
    {
        case -2: return '=';
        case -1: return '-';
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        default:
            std::cout << "Bad int value " << x << std::endl;
            throw 5;

    }
}

class snafu_t
{
public:
    explicit snafu_t(std::string in = "0")
    : m_value{digit_to_int(in.back())}
    {
        //std::cout << "Constructed with string " << in << " value " << m_value << std::endl;
        if(in.size() > 1)
        {
            in.pop_back();
            m_next = std::make_unique<snafu_t>(in);
        }
    }

    auto print() const -> std::string
    {
        //std::cout << "Printing with my value = " << m_value << std::endl;
        std::string ret;
        if(m_next)
            ret = m_next->print();
        ret.push_back(int_to_digit(m_value));

        return ret;
    }

    auto operator+=(snafu_t const & rhs) -> snafu_t &
    {
        // Start with least significant bit
        m_value += rhs.m_value;

        while(m_value > 2)
        {
            if(!m_next)
                m_next = std::make_unique<snafu_t>("1");
            else
                *m_next += snafu_t("1");
            m_value -= 5;
        }
        while(m_value < -2)
        {
            if(!m_next)
                m_next = std::make_unique<snafu_t>("-");
            else
                *m_next += snafu_t("-");
            m_value += 5;
        }

        // Now deal with more significant bits
        if(rhs.m_next)
        {
            if(!m_next)
                m_next = std::make_unique<snafu_t>(rhs.m_next->print());
            else
                *m_next += *rhs.m_next;
        }

        return *this;
    }

private:
    int m_value;
    std::unique_ptr<snafu_t> m_next;
};


int main()
{
    snafu_t total("0");
    std::string line;
    while(std::getline(std::cin, line))
    {
        snafu_t add{line};
        //std::cout << "Received " << line << " which should equal " << add.print() << std::endl;
        //std::cout << "Adding it to total " << total.print() << std::endl;
        total += add;
        //std::cout << "Total is now " << total.print() << std::endl;
    }

    std::cout << total.print() << std::endl;
    return 0;
}