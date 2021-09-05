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

/********************************************************************************
 * \file string.hpp
 * \brief Unicode string utilities
 ********************************************************************************/

#pragma once

#include "common.hpp"
#include "math.hpp"
#include "utf8.hpp"
#include "string_view.hpp"

#include <string>
#include <string_view>
#include <iostream>
#include <format>
#include <sstream>
#include <type_traits>

namespace Envy
{
    class string_view;

    /********************************************************************************
     * \brief UTF-8 String class
     * \see Envy::string_view
     ********************************************************************************/
    class string final
    {

        struct size_tag_t {};
        static constexpr size_tag_t size_tag {};
        string(size_tag_t, usize bytes);

    public:


        /********************************************************************************
         * \brief a
         ********************************************************************************/
        struct reserve_tag_t {};

        static constexpr reserve_tag_t reserve_tag {};


        /********************************************************************************
         * \brief Represents an invalid position
         ********************************************************************************/
        static constexpr usize npos { (usize) -1 };


    private: // Member data


        usize buffer_size;       ///< Size of buffer in bytes
        usize buffer_capacity;   ///< Capacity of buffer in bytes
        utf8::code_unit* buffer; ///< Pointer to buffer of UTF-8 code units

        mutable usize code_point_count {npos}; ///< Cached size of string in code points


    public: // Interface


        /********************************************************************************
         * \brief Constructs an empty string
         ********************************************************************************/
        string();


        /********************************************************************************
         * \brief Constructs an empty string, reserving a given number of bytes
         *
         * \param [in] bytes Number of bytes to reserve
         ********************************************************************************/
        string(reserve_tag_t, usize bytes);


        /********************************************************************************
         * \brief Constructs a string from a c string
         *
         * \param [in] cstr c string
         ********************************************************************************/
        string(const char* cstr);


        /********************************************************************************
         * \brief Constructs a string from c string
         *
         * \param [in] cstr c string
         ********************************************************************************/
        string(const char8_t* cstr);


        /********************************************************************************
         * \brief Constructs a string from a count and a fill character
         *
         * \param [in] count size of string
         * \param [in] fill fill character
         ********************************************************************************/
        string(usize count, char fill);


        /********************************************************************************
         * \brief Copy constructor
         *
         * \param [in] from Envy::string to copy from
         ********************************************************************************/
        string(const string& from);


        /********************************************************************************
         * \brief Move constructor
         *
         * \param [in] from Envy::string to move from
         ********************************************************************************/
        string(string&& from) noexcept;


        /********************************************************************************
         * \brief Copy construct from std::string
         *
         * \param [in] from std::string to copy from
         ********************************************************************************/
        string(const std::string& from);


        /********************************************************************************
         * \brief Constructs a string from a Envy::string_view implicitly
         *
         * \param [in] from Envy::string_view
         ********************************************************************************/
        string(string_view from);


        /********************************************************************************
         * \brief string's destructor
         *
         ********************************************************************************/
        ~string();


        /********************************************************************************
         * \brief Copy asignment
         *
         * \param [in] from Envy::string to copy from
         * \return Envy::string& reference to this
         ********************************************************************************/
        string& operator=(const string& from);


        /********************************************************************************
         * \brief Move asignment
         *
         * \param [in] from Envy::string to move from
         * \return Envy::string& reference to this
         ********************************************************************************/
        string& operator=(string&& from) noexcept;


        /********************************************************************************
         * \brief Copy asignment from std::string
         *
         * \param [in] from std::string to copy from
         * \return Envy::string& reference to this
         ********************************************************************************/
        string& operator=(const std::string& from);


        /********************************************************************************
         * \brief Implicit conversion to std::string_view
         *
         * \return std::string_view
         ********************************************************************************/
        operator std::string_view() const;


        /********************************************************************************
         * \brief Implicit conversion to std::string
         *
         * \return std::string
         ********************************************************************************/
        operator std::string() const;


        /********************************************************************************
         * \brief Returns a pointer to the first code unit
         *
         * \return const utf8::code_unit*
         ********************************************************************************/
        [[nodiscard]] const utf8::code_unit* data() const noexcept;


        /********************************************************************************
         * \brief Returns the size of the string in bytes
         *
         * \return usize Size in bytes
         ********************************************************************************/
        [[nodiscard]] usize size_bytes() const noexcept;


        // iterators


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


        // [[nodiscard]] const code_point* code_points() const;


        /********************************************************************************
         * \brief Returs the string as a string of code units
         *
         * \return std::basic_string_view<utf8::code_unit> code units
         ********************************************************************************/
        [[nodiscard]] std::basic_string_view<utf8::code_unit> code_units() const;


        /********************************************************************************
         * \brief Returns the string as a pointer to char
         *
         * \return const char* c string
         ********************************************************************************/
        [[nodiscard]] const char* c_str() const;


        // code_point code_point_at(usize i) const;


        /********************************************************************************
         * \brief Returns a sub string_view from the string with the range [first,last)
         *
         * \param [in] first First character of the view
         * \param [in] last One past the last character of the view
         * \return Envy::string_view
         ********************************************************************************/
        [[nodiscard]] string_view view(utf8::iterator first, utf8::iterator last) const noexcept;


        /********************************************************************************
         * \brief Returns a sub string_view from the string with the range [first,string.end())
         *
         * \param [in] first First character of the view
         * \return Envy::string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_from(utf8::iterator first) const noexcept;


        /********************************************************************************
         * \brief Returns a sub string_view from the string with the range [string.begin(),last)
         *
         * \param [in] last One past the last character of the view
         * \return Envy::string_view
         ********************************************************************************/
        [[nodiscard]] string_view view_until(utf8::iterator last) const noexcept;


        // string sub(utf8::iterator first, utf8::iterator last) const;
        // string sub(utf8::iterator first, usize count) const;


        /********************************************************************************
         * \brief Returns if the string is empty
         ********************************************************************************/
        [[nodiscard]] bool empty() const noexcept;


        /********************************************************************************
         * \brief Returns the size of the string in code points
         ********************************************************************************/
        [[nodiscard]] usize size() const noexcept(!Envy::debug);


        /********************************************************************************
         * \brief Returns the cpacity of the string in bytes
         *
         * \return usize Capacity
         ********************************************************************************/
        [[nodiscard]] usize capacity() const noexcept;


        /********************************************************************************
         * \brief Clears the string
         ********************************************************************************/
        void clear() noexcept;


        /********************************************************************************
         * \brief Request the buffer has a minimum capacity
         *
         * \param [in] bytes Number of bytes to reserve
         ********************************************************************************/
        void reserve(usize bytes);


        /********************************************************************************
         * \brief Appends a c string
         *
         * \param [in] cstr string to append
         ********************************************************************************/
        string& append(const char* cstr);


        /********************************************************************************
         * \brief Appends a string_view
         *
         * \param [in] str string_view to append
         ********************************************************************************/
        string& append(Envy::string_view str);


        /********************************************************************************
         * \brief Appends a code point
         *
         * \param [in] cp code point to append
         ********************************************************************************/
        string& append(utf8::code_point cp);


        /********************************************************************************
         * \brief Appends an ascii character
         *
         * \param [in] c character to append
         ********************************************************************************/
        string& append(char c);


        /********************************************************************************
         * \brief Appends a range
         *
         * \param [in] first iterator to first character
         * \param [in] last iterator to one past the last character
         * \return Envy::string& ref to this
         ********************************************************************************/
        template <std::forward_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
        requires requires(Envy::string s, typename std::iter_value_t<Iterator> v) { s.append(v); }
        string& append(Iterator first, Sentinel last)
        {
            for(;first < last; ++first)
            { this->append(*first); }
            return *this;
        }


        /********************************************************************************
         * \brief Appends a c string
         *
         * \param [in] cstr string to append
         ********************************************************************************/
        string& operator+=(const char* cstr);


        /********************************************************************************
         * \brief Appends a string_view
         *
         * \param [in] str string_view to append
         ********************************************************************************/
        string& operator+=(Envy::string_view str);


        /********************************************************************************
         * \brief Appends a code point
         *
         * \param [in] cp code point to append
         ********************************************************************************/
        string& operator+=(utf8::code_point cp);


        /********************************************************************************
         * \brief Appends an ascii character
         *
         * \param [in] c character to append
         ********************************************************************************/
        string& operator+=(char c);


        // iterator insert(utf8::iterator pos, utf8::iterator first, utf8::iterator last);
        // iterator insert(utf8::iterator pos, utf8::iterator first, usize count);
        // iterator insert(utf8::iterator pos, string_view str);

        // iterator remove(utf8::iterator pos);
        // iterator remove(utf8::iterator first, usize count);
        // iterator remove(utf8::iterator first, utf8::iterator last);

        // [[nodiscard]] iterator find(string_view s);
        // [[nodiscard]] iterator find(char c);
        // [[nodiscard]] iterator find(code_point cp);

        // [[nodiscard]] iterator find_last(string_view s);
        // [[nodiscard]] iterator find_last(char c);
        // [[nodiscard]] iterator find_last(code_point cp);

        // i32 replace(string_view target, string_view replacement);
        // i32 replace(utf8::iterator first, utf8::iterator last, string_view replacement);
        // i32 replace(utf8::iterator first, usize count, string_view replacement);

        // [[nodiscard]] i32 count(string_view sv) const;
        // [[nodiscard]] i32 count(char c) const;
        // [[nodiscard]] i32 count(code_point cp) const;


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

        // template <typename T>
        // T parse_as()
        // {
        //     // ...
        // }

        /********************************************************************************
         * \brief Equality compares two strings
         *
         * \return true if the strings are lexagraphically equivelant
         ********************************************************************************/
        [[nodiscard]] bool operator==(const Envy::string&) const noexcept;


        friend Envy::string operator+(Envy::string,Envy::string);

    private:

        [[nodiscard]] usize new_capacity(usize required_size) noexcept;
        void adjust_buffer(usize required_size);
    };


    static_assert(std::ranges::range<string>);
    static_assert(std::ranges::bidirectional_range<string>);


    Envy::string operator+(Envy::string,Envy::string);


    class format
    {
        std::string fmt;
    public:

        format(std::string_view fmt) : fmt{fmt} {}

        template <typename ... Ts>
        std::string operator()(Ts&& ... args)
        { return std::format(fmt, std::forward<Ts>(args)... ); }
    };


    [[deprecated]] [[nodiscard]]
    std::string replace(std::string_view str, std::string_view target, std::string_view replacement);


    template <typename T>
    concept stream_insertable = requires (T t) { std::cout << t; };


    template <typename T>
    concept tostring_member = requires (T t) { { t.to_string() } -> std::convertible_to<std::string>; };


    template <typename T>
    concept tostring_freefunc = requires (T t) { { ::to_string(t) } -> std::convertible_to<std::string>; };


    template <typename T>
    concept convertable_to_string =
        std::convertible_to<T,std::string>  ||
        numeric<T>                          ||
        stream_insertable<T>                ||
        tostring_member<T>                  ||
        tostring_freefunc<T>;


    template <convertable_to_string T>
    [[nodiscard]] std::string to_string(T&& v)
    {

        if constexpr (std::same_as<std::remove_cvref_t<T>,bool>)
        { return v ? "true" : "false"; }

        else if constexpr (std::same_as<std::remove_cvref_t<T>,char>)
        { return std::string(&v,1); }

        // TODO: unicode characters

        else if constexpr (std::convertible_to<T,std::string>)
        { return (std::string) std::forward<T>(v); }

        else if constexpr (numeric<std::remove_cvref_t<T>>)
        { return std::to_string(v); }

        else if constexpr (tostring_member<T>)
        { return std::forward<T>(v).to_string(); }

        else if constexpr (tostring_freefunc<T>)
        { return ::to_string(std::forward<T>(v)); }

        else if constexpr (stream_insertable<T>)
        {
            std::stringstream ss;
            ss << std::forward<T>(v);
            return std::move(ss).str();
        }

        else { static_assert(true,"wat?"); return std::string(); }
    }

}

[[nodiscard]] inline Envy::format operator ""_f (const char * fmt, std::size_t len)
{ return Envy::format(fmt); }

namespace std
{
    template <>
    struct hash<Envy::string>
    {
        std::size_t operator()(const Envy::string& s) const noexcept
        {
            return std::hash<std::basic_string_view<Envy::utf8::code_unit>>{}( { s.data() , s.size_bytes() } );
        }
    };
}