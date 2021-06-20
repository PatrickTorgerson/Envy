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

#include "unicode.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <cstring>
#include <algorithm>

namespace Envy
{

    template <character_encoding E>
    class basic_string
    {
    public:

        using encoding = E;
        using code_unit = encoding::code_unit;
        using buffer = encoding::buffer;

        class character
        {
            friend class iterator;
            friend class basic_string<encoding>;

            basic_string<encoding>& s;
            code_unit* ptr;

            character(basic_string<encoding>& s, code_unit* ptr) noexcept : s{s} , ptr{ptr} {}
        public:

            // ? no copy, no move, private construction ?

            character& operator=(const character& c)
            {
                s.set_char(ptr, c);
                return *this;
            }
            character& operator=(char c)
            {
                s.set_char(ptr, c);
                return *this;
            }
            character& operator=(code_point cp)
            {
                s.set_char(ptr, cp);
                return *this;
            }

            explicit operator code_point() const
            {
                return encoding::decode(ptr);
            }

            explicit operator char() const
            {
                // TODO: assert( ascii::valid(ptr), "character is not ascii")
                return (char) *ptr;
            }

        };


        class iterator
        {
        public:

        private:
            basic_string<encoding>* s;
            code_unit* ptr;
        public:

            // // Member Types
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = character;
            using pointer           = value_type*;  // or also value_type*
            using reference         = value_type&;  // or also value_type&

            iterator() noexcept : s{nullptr}, ptr{nullptr} {}

            iterator(basic_string<encoding>* s, code_unit* ptr) noexcept : s{s} , ptr{ptr} {}

            iterator(const iterator&) = default;
            iterator& operator=(const iterator&) = default;

            iterator& operator++() noexcept
            {
                encoding::increment(&ptr);
                return *this;
            }

            iterator& operator++(int) noexcept
            {
                iterator t {*this};
                ++(*this);
                return t;
            }

            iterator& operator--() noexcept
            {
                encoding::decrement(&ptr);
                return *this;
            }

            iterator& operator--(int) noexcept
            {
                iterator t {*this};
                --(*this);
                return t;
            }

            character operator*()
            {
                return {*s,ptr};
            }

            const character operator*() const
            {
                return {*s,ptr};
            }

            bool operator==(const iterator& other) const noexcept
            { return ptr == other.ptr; }

            code_unit* get()
            { return ptr; }
        };

         // static_assert(std::bidirectional_iterator<iterator>);

    private:

        std::vector<code_unit> data;

        usize size_chars;

        static constexpr usize npos { (usize) -1 };

    public:

        basic_string() = default;

        // temp
        basic_string(const char* cstr) :
            data {cstr, cstr + strlen(cstr) + 1}
        {}

        basic_string(const basic_string<encoding>&) = default;
        basic_string(basic_string<encoding>&&) = default;

        basic_string<encoding>& operator=(const basic_string<encoding>&) = default;
        basic_string<encoding>& operator=(basic_string<encoding>&&) = default;

        constexpr iterator begin() noexcept { return {this,data.data()}; }
        // constexpr const_iterator begin() const noexcept;
        // constexpr const_iterator cbegin() const noexcept;
        constexpr iterator end() noexcept { return {this,data.data()+data.size()-1}; }
        // constexpr const_iterator end() const noexcept;
        // constexpr const_iterator cend() const noexcept;

        // constexpr reverse_iterator       rbegin() noexcept;
        // constexpr const_reverse_iterator rbegin() const noexcept;
        // constexpr const_reverse_iterator crbegin() const noexcept;
        // constexpr reverse_iterator       rend() noexcept;
        // constexpr const_reverse_iterator rend() const noexcept;
        // constexpr const_reverse_iterator crend() const noexcept;

        char* cstr() noexcept
        { return (char*) data.data(); }
        const char* cstr() const noexcept
        { return (const char*) data.data(); }

        // code_unit* data() noexcept;
        // const code_unit* data() const noexcept;

        // usize data_size() const;

        void set_char(iterator it, char c)
        { set_char(it.get(),c); }

        void set_char(iterator it, character c)
        { set_char(it.get(),c); }

        void set_char(code_unit* p, char c)
        {
            auto psz {encoding::count_code_units(p)};

            if(psz > 1)
            {
                std::shift_left(p+1, data.data()+data.size(), psz-1);
            }

            *p = c;
        }

        void set_char(code_unit* p, character c)
        {
            // auto psz {encoding::count_code_units(p)};
            // auto csz {encoding::count_code_units(c.ptr)};
            // auto diff = csz - psz;

            // if(diff > 0 && size_bytes + diff > capacity)
            // { realloc(diff); }

            // auto suffix { ((usize) p - data.get()) + psz };
            // Envy::mutable_buffer(data.get(),size_bytes).shiftr(suffix, size_bytes - suffix, diff);

            // for(int i {}; i < csz; ++i)
            // {
            //     p[i] = c.ptr[i];
            // }
        }

    };

    template<character_encoding E>
    std::ostream& operator<<(std::ostream& os, const basic_string<E>& s)
    {
        os << s.cstr();
        return os;
    }

}