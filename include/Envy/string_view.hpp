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
         * \brief Constructs a null string_view
         *
         ********************************************************************************/
        string_view() = default;


        /********************************************************************************
         * \brief Constructs a string_view from a c string
         *
         * \param [in] data c string
         ********************************************************************************/
        string_view(const char* data) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from a Envy::string
         *
         * \param [in] str Envy::string
         ********************************************************************************/
        string_view(const Envy::string& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from a std::string
         *
         * \param [in] str std::string
         ********************************************************************************/
        string_view(const std::string& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from a std::string_view
         *
         * \param [in] str std::string_view
         ********************************************************************************/
        string_view(const std::string_view& str) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from a code_unit buffer
         *
         * \param [in] data Valid UTF-8 buffer
         ********************************************************************************/
        string_view(const utf8::code_unit* data) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from a c string and a size
         ********************************************************************************/
        string_view(const char* data, usize size) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view
         ********************************************************************************/
        string_view(const utf8::code_unit* data, usize size) noexcept;


        /********************************************************************************
         * \brief Constructs a string_view from an iterator pair
         *
         * \param [in] first first character of view
         * \param [in] last one past las character of view
         ********************************************************************************/
        string_view(utf8::iterator first, utf8::iterator last) noexcept;


        /********************************************************************************
         * \brief Implicit convertion to std::string_view
         *
         * \return std::string_view
         ********************************************************************************/
        operator std::string_view() const;


        /********************************************************************************
         * \brief Returns the data buffer
         *
         * Because a string_view can view a substring of a string the null-terminator
         * will be the end of the viewed string but not necesarily the view. That is to say,
         * `strlen(sv.data()) != sv.size_bytes()`
         *
         * \return const utf8::code_unit*
         ********************************************************************************/
        [[nodiscard]] const utf8::code_unit* data() const noexcept;


        /********************************************************************************
         * \brief size of viewed buffer in bytes
         ********************************************************************************/
        [[nodiscard]] usize size_bytes() const noexcept;


        /********************************************************************************
         * \brief Returs the string as a string of code units
         ********************************************************************************/
        [[nodiscard]] std::basic_string_view<utf8::code_unit> code_units() const;


        /********************************************************************************
         * \brief Returns an iterator pointing to the first character
         ********************************************************************************/
        [[nodiscard]] utf8::iterator begin() const noexcept;


        /********************************************************************************
         * \brief Returns an iterator pointing to one past the last character
         ********************************************************************************/
        [[nodiscard]] utf8::iterator end() const noexcept;


        /********************************************************************************
         * \brief Returns an iterator pointing to the first character
         ********************************************************************************/
        [[nodiscard]] utf8::iterator cbegin() const noexcept;


        /********************************************************************************
         * \brief Returns an iterator pointing to one past the last character
         ********************************************************************************/
        [[nodiscard]] utf8::iterator cend() const noexcept;


        /********************************************************************************
         * \brief Returns a reverse iterator pointing to the last character
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator rbegin() const noexcept;


        /********************************************************************************
         * \brief Returns a reverse iterator pointing to one before the first character
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator rend() const noexcept;


        /********************************************************************************
         * \brief Returns a reverse iterator pointing to the last character
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator crbegin() const noexcept;


        /********************************************************************************
         * \brief Returns a reverse iterator pointing to one before the first character
         ********************************************************************************/
        [[nodiscard]] utf8::reverse_iterator crend() const noexcept;


        /********************************************************************************
         * \brief Returns if the view is empty
         ********************************************************************************/
        [[nodiscard]] bool empty() const noexcept;


        /********************************************************************************
         * \brief Returns the size of the view in code points
         ********************************************************************************/
        [[nodiscard]] usize size() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief Returns the first character in the view
         ********************************************************************************/
        [[nodiscard]] utf8::code_point front() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief Returns the last character in the view
         ********************************************************************************/
        [[nodiscard]] utf8::code_point back() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief Returns a sub string_view from the view with the range [first,last)
         *
         * \param [in] first First character of the view
         * \param [in] last One past the last character of the view
         * \return Envy::string_view
         ********************************************************************************/
        [[nodiscard]] string_view view(utf8::iterator first, utf8::iterator last) const noexcept;


        /********************************************************************************
         * \brief Returns a sub string_view from the view with the range [first,view.end())
         *
         * \param [in] first First character of the view
         * \return Envy::string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_from(utf8::iterator first) const noexcept;


        /********************************************************************************
         * \brief Returns a sub string_view from the view with the range [view.begin(),last)
         *
         * \param [in] last One past the last character of the view
         * \return string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_until(utf8::iterator last) const noexcept;


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