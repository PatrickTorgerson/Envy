#include <utf8.hpp>

namespace Envy::utf8
{

    int bytes(char8 lead)
    {
        int units {std::countl_one(lead)};
        // TODO: assert(units != 1 && units < 5, "Invalid UTF-8");
        return units + (units?0:1);
    }


    int bytes(code_point cp)
    {
        int code_units {1};
        if(cp > 0x007F) ++code_units;
        if(cp > 0x07FF) ++code_units;
        if(cp > 0xFFFF) ++code_units;
        return code_units;
    }


    bool lead_byte(char8 b) noexcept
    {
        int ones {std::countl_one(b)};
        return ones != 1 && ones < 5;
    }


    bool continuation_byte(char8 b) noexcept
    { return (b >> 6) == 2; }


    bool valid_char(char8* lead) noexcept
    {
        int units {std::countl_one(*lead)};

        if(units == 1 || units > 4)
        { return false; }

        units -= (units?1:0);
        ++lead;

        for(int i {}; i < units; ++i)
        {
            if(!continuation_byte(*lead))
            { return false; }
            ++lead;
        }

        return true;
    }


    void increment(char8** lead_ptr)
    {
        *lead_ptr += bytes(**lead_ptr);
    }


    void decrement(char8** lead_ptr)
    {
        do { --(*lead_ptr); } while(continuation_byte(**lead_ptr));
        // TODO: assert((b - *bp) < 5, "Invalid UTF-8");
    }

}