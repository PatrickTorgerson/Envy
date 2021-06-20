///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2021 Patrick Torgerson
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.hpp"

#include <concepts>
#include <type_traits>
#include <bit>
#include <ranges>

namespace Envy
{

    using code_point = u32;

    template <typename T>
    concept character_encoding =
    requires
    {
        typename T::code_unit;
        typename T::buffer;
    }
    &&
    requires(typename T::buffer buff, typename T::buffer* buffptr, code_point cp)
    {
        { T::decode(buff) } -> std::same_as<code_point>;

        { T::encode(buff, cp) };

        { T::increment(buffptr) };
        { T::decrement(buffptr) };

        { T::count_code_units(buff) } -> std::same_as<int>;

        { T::code_units_required(cp) } noexcept -> std::same_as<int>;

        { T::first_code_unit(buff) } noexcept -> std::same_as<bool>;

        { T::valid(buff) } noexcept -> std::same_as<bool>;
    };


    class utf8
    {
    public:

        using code_unit  = u8;
        using buffer     = code_unit*;

        static constexpr code_unit  c_mask {0b00111111};
        static constexpr code_unit f2_mask {0b00011111};
        static constexpr code_unit f3_mask {0b00001111};
        static constexpr code_unit f4_mask {0b00000111};

        static constexpr code_unit  c_prefix {0b10000000};
        static constexpr code_unit f2_prefix {0b11000000};
        static constexpr code_unit f3_prefix {0b11100000};
        static constexpr code_unit f4_prefix {0b11110000};


        static void increment(buffer* bp)
        { *bp += count_code_units(*bp); }


        static void decrement(buffer* bp)
        {
            do { --(*bp); } while(continuation_unit(**bp));

            // TODO: assert((b - *bp) < 5, "Invalid UTF-8");
        }


        static void encode(buffer b, code_point cp)
        {

            // TODO: assert(*b != (code_unit) '\0', "Insufficient space in buffer")

            int continuation_units { code_units_required(cp) - 1 };

            // encode first code_unit
            switch(continuation_units)
            {
                case 0: *b = (code_unit) cp; return;

                case 1: *b = f2_prefix | ((code_unit)(cp >> 6)  & f2_mask); break;
                case 2: *b = f3_prefix | ((code_unit)(cp >> 12) & f3_mask); break;
                case 3: *b = f4_prefix | ((code_unit)(cp >> 18) & f4_mask); break;
            }

            // encode continuation units
            for(int i {1}; i <= continuation_units; ++i)
            {
                ++b;

                // TODO: assert(*b != (code_unit) '\0', "Insufficient space in buffer")

                int bits { 6 * (continuation_units - i) };

                *b = c_prefix | ((code_unit)(cp >> bits) & c_mask);
            }
        }


        static code_point decode(buffer b)
        {
            code_point cp {};

            int units { count_code_units(b) };

            switch(units)
            {
                case 1: return (code_point) *b;

                case 2: cp = (code_point)(*b & f2_mask); break;
                case 3: cp = (code_point)(*b & f3_mask); break;
                case 4: cp = (code_point)(*b & f4_mask); break;
            }

            ++b;

            while(continuation_unit(*b))
            {
                // TODO: assert(*b != (code_unit) '\0', "Invalid UTF-8, character missing code_units")

                cp <<= 6;

                cp |= (code_point)(*b & c_mask);

                ++b;
            }

            return cp;
        }


        static int count_code_units(buffer b)
        {
            int units {std::countl_one(*b)};
            // TODO: assert(units != 1 && units < 5, "Invalid UTF-8");
            return units + (units?0:1);
        }


        static int code_units_required(code_point cp) noexcept
        {
            int code_units {1};
            if(cp > 0x007F) ++code_units;
            if(cp > 0x07FF) ++code_units;
            if(cp > 0xFFFF) ++code_units;
            return code_units;
        }


        static constexpr bool first_code_unit(buffer b) noexcept
        {
            int ones {std::countl_one(*b)};
            return ones != 1 && ones < 5;
        }


        static constexpr bool valid(buffer b) noexcept
        {
            int units {std::countl_one(*b)};

            if(units == 1 || units > 4)
            { return false; }

            units -= (units?1:0);
            ++b;

            for(int i {}; i < units; ++i)
            {
                if(!continuation_unit(*b))
                { return false; }
                ++b;
            }

            return true;
        }

    private:

        static constexpr bool continuation_unit(code_unit u) noexcept
        { return (u >> 6) == 2; }
    };

    static_assert(character_encoding<utf8>, "Envy::utf8 is out of spec");

}