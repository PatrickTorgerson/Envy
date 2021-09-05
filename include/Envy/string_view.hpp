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

    // TODO: Verify all operation are safe on a null string_view

    class string_view final
    {

        usize size_{0};
        const utf8::code_unit* ptr_{nullptr};

        mutable usize code_point_count;

    public:

        /********************************************************************************
         * \brief Constructs a string view
         *
         ********************************************************************************/
        string_view() = default;

        // implicit converting constructors


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] data
         ********************************************************************************/
        string_view(const char* data) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] str
         ********************************************************************************/
        string_view(const Envy::string& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] str
         ********************************************************************************/
        string_view(const std::string& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] str
         ********************************************************************************/
        string_view(const std::string_view& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] data
         ********************************************************************************/
        string_view(const utf8::code_unit* data) noexcept;

        // size is in bytes, not characters, be carefull not to cut a char in twine
        // recomended you use Envy::string::view()
        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] data
         * \param [in] size
         ********************************************************************************/
        string_view(const char* data, usize size) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] data
         * \param [in] size
         ********************************************************************************/
        string_view(const utf8::code_unit* data, usize size) noexcept;


        /********************************************************************************
         * \brief Constructs a string view
         *
         * \param [in] first
         * \param [in] last
         ********************************************************************************/
        string_view(utf8::iterator first, utf8::iterator last) noexcept;


        /********************************************************************************
         * \brief
         *
         * \return std::string_view
         ********************************************************************************/
        operator std::string_view() const;


        /********************************************************************************
         * \brief
         *
         * \return const utf8::code_unit*
         ********************************************************************************/
        [[nodiscard]] const utf8::code_unit* data() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return usize
         ********************************************************************************/
        [[nodiscard]] usize size_bytes() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return std::basic_string_view<utf8::code_unit>
         ********************************************************************************/
        [[nodiscard]] std::basic_string_view<utf8::code_unit> code_units() const;

        /********************************************************************************
         * \brief
         *
         * \return utf8::iterator
         ********************************************************************************/
        [[nodiscard]] utf8::iterator begin() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::iterator
         ********************************************************************************/
        [[nodiscard]] utf8::iterator end() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::iterator
         ********************************************************************************/
        [[nodiscard]] utf8::iterator cbegin() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::iterator
         ********************************************************************************/
        [[nodiscard]] utf8::iterator cend() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::reverse_iterator
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator rbegin() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::reverse_iterator
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator rend() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::reverse_iterator
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator crbegin() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return utf8::reverse_iterator
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator crend() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return true
         * \return false
         ********************************************************************************/
        [[nodiscard]] bool empty() const noexcept;


        /********************************************************************************
         * \brief
         *
         * \return usize
         ********************************************************************************/
        [[nodiscard]] usize size() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief
         *
         * \return utf8::code_point
         ********************************************************************************/
        [[nodiscard]] utf8::code_point front() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief
         *
         * \return utf8::code_point
         ********************************************************************************/
        [[nodiscard]] utf8::code_point back() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief
         *
         * \param [in] first
         * \param [in] last
         * \return string_view
         ********************************************************************************/
        [[nodiscard]] string_view view(utf8::iterator first, utf8::iterator last) const;


        /********************************************************************************
         * \brief
         *
         * \param [in] first
         * \return string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_from(utf8::iterator first) const;


        /********************************************************************************
         * \brief
         *
         * \param [in] last
         * \return string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_until(utf8::iterator last) const;


        /********************************************************************************
         * \brief Determines if a string contains a given substring
         *
         * \param [in] sv Substring to find
         * \return true if *sv* is a substring of the string
         ********************************************************************************/
        [[nodiscard]] bool contains(string_view sv) const noexcept;


        /********************************************************************************
         * \brief Determines if a string contains a given code point
         *
         * \param [in] cp Code point to search for
         * \return true is string contains *cp*
         ********************************************************************************/
        [[nodiscard]] bool contains(utf8::code_point cp) const noexcept;


        /********************************************************************************
         * \brief Determines if a string contains any of the characters in *sv*
         *
         * \param [in] sv Characters to search for
         * \return true if any character in *sv* is found in the string
         ********************************************************************************/
        [[nodiscard]] bool contains_any(string_view sv) const noexcept;


        /********************************************************************************
         * \brief Determines if a string contains all of the characters in *sv*
         *
         * \param [in] sv Characters to search for
         * \return true if all characters in *sv* were found in the string
         ********************************************************************************/
        [[nodiscard]] bool contains_all(string_view sv) const noexcept;


        /********************************************************************************
         * \brief Determines if a string contains only characters found in *sv*
         *
         * \param [in] sv Characters to search for
         * \return true if all characters in the string were found in *sv*
         ********************************************************************************/
        [[nodiscard]] bool contains_only(string_view sv) const noexcept;


        /********************************************************************************
         * \brief Equality compare two string_view 's
         *
         * \return true
         * \return false
         ********************************************************************************/
        [[nodiscard]] bool operator==(const Envy::string_view&) const noexcept;
    };

}

// Hash support for Envy::string_view
namespace std
{
    template <>
    struct hash<Envy::string_view>
    {
        std::size_t operator()(const Envy::string_view& s) const noexcept
        {
            return std::hash<std::basic_string_view<Envy::utf8::code_unit>>{}( { s.data() , s.size_bytes() } );
        }
    };
}