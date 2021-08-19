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
#include "utf8.hpp"

#include <string>
#include <string_view>


namespace Envy
{
    class string;

    class string_view final
    {

        usize size_{0};
        const utf8::code_unit* ptr_{nullptr};

        mutable usize code_point_count;

    public:

        string_view() = default;

        // implicit converting constructors
        string_view(const Envy::string& str) noexcept;
        string_view(const std::string& str) noexcept;
        string_view(const std::string_view& str) noexcept;
        string_view(const utf8::code_unit* data) noexcept;

        // size is in bytes, not characters, be carefull not to cut a char in twine
        // reccomended you use Envy::string::view()
        string_view(const char* data, usize size) noexcept;
        string_view(const utf8::code_unit* data, usize size) noexcept;
        string_view(utf8::iterator first, utf8::iterator last) noexcept;

        [[nodiscard]] const utf8::code_unit* data() const noexcept;
        [[nodiscard]] usize size_bytes() const noexcept;

        [[nodiscard]] std::basic_string_view<utf8::code_unit> code_units() const;

        // iterators
        [[nodiscard]] utf8::iterator begin() const noexcept;
        [[nodiscard]] utf8::iterator end() const noexcept;
        [[nodiscard]] utf8::iterator cbegin() const noexcept;
        [[nodiscard]] utf8::iterator cend() const noexcept;
        [[nodiscard]] utf8::reverse_iterator rbegin() const noexcept;
        [[nodiscard]] utf8::reverse_iterator rend() const noexcept;
        [[nodiscard]] utf8::reverse_iterator crbegin() const noexcept;
        [[nodiscard]] utf8::reverse_iterator crend() const noexcept;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] usize size() const noexcept(!Envy::debug);

        [[nodiscard]] bool operator==(const Envy::string_view&) const noexcept;
    };

}