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

#include <string>
#include <format>
#include <iterator>
#include <type_traits>
#include <bit>

namespace Envy
{

    template <typename T>
    requires std::same_as<std::remove_const_t<T>, u32> // T can be u32 or const u32
    class utf8_iterator final
    {
        u8* bytes;
    public:

        // Member Types
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using mutval            = std::remove_const_t<value_type>;
        using pointer           = value_type*;  // or also value_type*
        using reference         = value_type&;  // or also value_type&


        // Construction
        utf8_iterator() noexcept : bytes {nullptr} {}

        explicit utf8_iterator(u8* it) noexcept : bytes {it} {}

        utf8_iterator(const utf8_iterator&) = default;

        utf8_iterator& operator=(const utf8_iterator&) = default;


        // increment

        utf8_iterator& operator++() noexcept
        {
            difference_type offset { std::countl_one(*bytes) };
            offset = offset ? offset : 1u;
            bytes += offset;
            return *this;
        }

        utf8_iterator  operator++(int) noexcept
        {
            utf8_iterator t {*this};
            ++(*this);
            return t;
        }

        utf8_iterator& operator--() noexcept
        {
            do { --bytes; } while(continuation_byte(*bytes));
        }

        utf8_iterator  operator--(int) noexcept
        {
            utf8_iterator t {*this};
            --(*this);
            return t;
        }

        mutval operator*() const
        {
            mutval cp {};
            u8* b {bytes};

            constexpr u8  c_mask {0b00111111};
            constexpr u8 f2_mask {0b00011111};
            constexpr u8 f3_mask {0b00001111};
            constexpr u8 f4_mask {0b00000111};

            int ones { std::countl_one(*b) };

            switch(ones)
            {
                case 0: return (mutval) *b;
                case 2: cp = (mutval)(*b & f2_mask); break;
                case 3: cp = (mutval)(*b & f3_mask); break;
                case 4: cp = (mutval)(*b & f4_mask); break;
                default: throw false; // invalic utf-8
            }

            ++b;

            while(continuation_byte(*b))
            {
                cp <<= 6;

                cp |= (mutval)(*b & c_mask);

                ++b;
            }

            return cp;
        }

        bool operator==(const utf8_iterator& other) const noexcept
        {
            return bytes == other.bytes;
        }

    private:

        constexpr bool continuation_byte(u8 b) const noexcept
        { return (b >> 6) == 2; }

    };

    static_assert(std::bidirectional_iterator<utf8_iterator<u32>>);
    static_assert(std::bidirectional_iterator<utf8_iterator<const u32>>);

}