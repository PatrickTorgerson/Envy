#include <utf8.hpp>
#include <log.hpp>

#include <cstring>
#include <limits>

namespace Envy::utf8
{

    // ==== Envy::utf8::code_point ====

    code_point::code_point(code_unit value) noexcept :
        cp { static_cast<u32>(value) }
    {}


    code_point::code_point(char value) noexcept :
        cp { static_cast<u32>(value) }
    {}


    code_point::code_point(char8_t value) noexcept :
        cp { static_cast<u32>(value) }
    {}


    code_point::code_point(u32 value) noexcept :
        cp {value}
    {}


    code_point::code_point(const char* value) noexcept :
        cp { decode( (const code_unit*) value).get() }
    {
        Envy::debug_assert(utf8::count_code_points( (const code_unit*) value) == 1, "Trying to construct EnvyLLutf8::code_point from multi character string");
    }


    u32& code_point::get() noexcept
    { return cp; }


    code_point::operator u32() const noexcept
    { return cp; }


    std::ostream& operator<<(std::ostream& os, code_point cp)
    {
        code_unit buffer[5] {'\0','\0','\0','\0','\0'};

        encode(cp, buffer);

        os << buffer;

        return os;
    }


    // ==== utf8 helper funcs ====


    namespace
    {
        // these are used to mask off the encoding bits of the code units so we can get at that jucy unicode code point data
        constexpr code_unit  c_mask {0b00111111}; // continuation unit
        constexpr code_unit l2_mask {0b00011111}; // lead unit indicating 2 continuation units
        constexpr code_unit l3_mask {0b00001111}; // lead unit indicating 3 continuation units
        constexpr code_unit l4_mask {0b00000111}; // lead unit indicating 4 continuation units

        // prefixes for unicode code units, used for encoding code points
        constexpr code_unit  c_prefix {0b10000000}; // continuation unit
        constexpr code_unit f2_prefix {0b11000000}; // lead unit indicating 2 continuation units
        constexpr code_unit f3_prefix {0b11100000}; // lead unit indicating 3 continuation units
        constexpr code_unit f4_prefix {0b11110000}; // lead unit indicating 4 continuation units

        // All continuation units contain exactly six bits from the code point
        constexpr i32 bits_per_continuation_unit {6};
    }


    i32 code_units_encoded(const code_unit* lead) noexcept(!Envy::debug)
    {
        Envy::debug_assert(is_lead_unit(lead), "Invalid UTF-8");

        i32 units {std::countl_one(*lead)};
        return units + (units?0:1);
    }


    i32 code_units_required(code_point cp) noexcept
    {
        i32 code_units {1};
        if( static_cast<u32>(cp) > 0x007Fu) ++code_units;
        if( static_cast<u32>(cp) > 0x07FFu) ++code_units;
        if( static_cast<u32>(cp) > 0xFFFFu) ++code_units;
        return code_units;
    }


    i32 count_code_points(const code_unit* buffer) noexcept(!Envy::debug)
    {
        i32 chars {};
        while(*buffer != '\0')
        {
            increment_ptr(&buffer);
            ++chars;
        }
        return chars;
    }


    i32 count_code_points(const code_unit* buffer, usize size_bytes) noexcept(!Envy::debug)
    {
        i32 chars {};
        const code_unit* end {buffer + size_bytes};

        while(*buffer != '\0' && buffer < end)
        {
            increment_ptr(&buffer);
            ++chars;
        }
        return chars;
    }


    usize size_bytes(const code_unit* buffer) noexcept
    {
        return strlen( reinterpret_cast<const char*>(buffer) );
    }

    usize size_bytes(const char* buffer) noexcept
    {
        return strlen( buffer );
    }


    bool is_lead_unit(const code_unit* unit) noexcept
    {
        // must begin with 0, 2, 3, or 4 ones
        i32 ones {std::countl_one(*unit)};
        return ones != 1 && ones < 5;
    }


    bool is_continuation_unit(const code_unit* unit) noexcept
    {
        // first two bits must be '10'
        return (*unit >> bits_per_continuation_unit) == 0b10;
    }


    bool is_valid_char(const code_unit* lead) noexcept
    {
        int units {std::countl_one(*lead)};

        if(units == 1 || units > 4)
        { return false; }

        units -= (units?1:0);

        ++lead;

        for(int i {}; i < units; ++i)
        {
            if(!is_continuation_unit(lead))
            { return false; }
            ++lead;
        }

        return true;
    }


    void increment_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug)
    {
        *lead_ptr += code_units_encoded(*lead_ptr);
    }


    void decrement_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug)
    {
        do { --(*lead_ptr); } while (is_continuation_unit(*lead_ptr));
    }


    void increment_ptr(code_unit** lead_ptr) noexcept(!Envy::debug) { increment_ptr((const code_unit**)lead_ptr); }
    void decrement_ptr(code_unit** lead_ptr) noexcept(!Envy::debug) { decrement_ptr((const code_unit**)lead_ptr); }


    code_point decode(const code_unit* lead) noexcept(!Envy::debug)
    {
        Envy::debug_assert(lead != nullptr, "Trying to decode nullptr");
        Envy::debug_assert(is_lead_unit(lead), "Trying to decode invalid UTF-8");

        u32 codepoint {};

        i32 units {code_units_encoded(lead)};

        switch(units)
        {
            // ascii
            case 1: return code_point {*lead};

            // mask off encoding data
            case 2: codepoint = static_cast<code_unit>(*lead & l2_mask); break;
            case 3: codepoint = static_cast<code_unit>(*lead & l3_mask); break;
            case 4: codepoint = static_cast<code_unit>(*lead & l4_mask); break;
        }

        ++lead;
        --units;

        while(units > 0)
        {
            Envy::debug_assert(is_continuation_unit(lead), "Invalid UTF-8, character missing code_units");

            // append data to code unit
            codepoint <<= bits_per_continuation_unit;
            codepoint |= static_cast<u32>(*lead & c_mask);

            ++lead;
            --units;
        }

        return code_point {codepoint};
    }


    void encode(code_point cp, code_unit* ptr) noexcept(!Envy::debug)
    {
        Envy::debug_assert(lead != nullptr, "Trying to encode to nullptr");

        const u32 codepoint {cp.get()};

        const i32 continuation_units { code_units_required(cp) - 1 };
        const i32 continuation_bits  { bits_per_continuation_unit * continuation_units };

        // encode first code_unit
        switch(continuation_units)
        {
            // ascii
            case 0: *ptr = (code_unit) cp.get(); return;

            case 1: *ptr = f2_prefix | (static_cast<code_unit>(codepoint >> continuation_bits) & l2_mask); break;
            case 2: *ptr = f3_prefix | (static_cast<code_unit>(codepoint >> continuation_bits) & l3_mask); break;
            case 3: *ptr = f4_prefix | (static_cast<code_unit>(codepoint >> continuation_bits) & l4_mask); break;
        }

        // encode continuation units
        for(i32 i {1}; i <= continuation_units; ++i)
        {
            ++ptr;

            // number of bits we need to shif to get to this code unit's data
            i32 bits { bits_per_continuation_unit * (continuation_units - i) };

            *ptr = c_prefix | ( static_cast<code_unit>(codepoint >> bits) & c_mask );
        }
    }


    code_point next_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug)
    {
        code_point cp {decode(*lead_ptr)};
        increment_ptr(lead_ptr);
        return cp;
    }


    code_point prev_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug)
    {
        code_point cp {decode(*lead_ptr)};
        decrement_ptr(lead_ptr);
        return cp;
    }


    bool is_valid_utf8(const code_unit* buffer) noexcept
    {
        if(*buffer == '\0')
        { return true; }

        if(!is_lead_unit(buffer))
        { return false; }

        while(*buffer != '\0')
        {
            if(!is_valid_char(buffer))
            { return false; }

            //increment buffer
            i32 units { std::countl_one(*buffer) };
            buffer += units + (units?0:1);
        }

        return true;
    }


    // ==== Envy::utf8::iterator ====


    iterator::iterator() noexcept :
        ptr {nullptr}
    {}

    iterator::iterator(const code_unit* p) noexcept :
        ptr {p}
    {}


    iterator& iterator::operator++() noexcept(!Envy::debug)
    {
        increment_ptr(&ptr);
        return *this;
    }


    iterator iterator::operator++(int) noexcept(!Envy::debug)
    {
        iterator i {*this};
        increment_ptr(&ptr);
        return i;
    }


    iterator& iterator::operator--() noexcept(!Envy::debug)
    {
        decrement_ptr(&ptr);
        return *this;
    }


    iterator iterator::operator--(int) noexcept(!Envy::debug)
    {
        iterator i {*this};
        decrement_ptr(&ptr);
        return i;
    }


    iterator::value_type iterator::operator*() const noexcept(!Envy::debug)
    {
        return decode(ptr);
    }


    bool iterator::operator==(const iterator& other) const noexcept
    {
        return ptr == other.ptr;
    }


    const code_unit* iterator_ptr(const iterator& i) noexcept
    { return i.ptr; }


    usize iterator_distance_bytes(const iterator& i1, const iterator& i2) noexcept
    {
        return (usize) (i1.ptr - i2.ptr);
    }

}