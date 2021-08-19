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
#include "math.hpp"
#include "utf8.hpp"
#include "string_view.hpp"

#include <string>
#include <string_view>
#include <iostream>
#include <format>
#include <sstream>
#include <type_traits>
#include <ranges>

namespace Envy
{
    class string_view;
    //class ascii_string;


    class string final
    {
    public:

        static constexpr usize npos { (usize) -1 };

        struct reserve_tag {};
        struct size_tag {};

    private:

        usize buffer_size;
        usize buffer_capacity;
        utf8::code_unit* buffer;

        mutable usize code_point_count;

    public:
        string();

        string(reserve_tag, usize bytes);
        string(size_tag, usize bytes);

        string(const char* cstr);
        string(const char8_t* cstr);
        string(usize reserved, char fill);

        ~string();

        [[nodiscard]] const utf8::code_unit* data() const noexcept;
        [[nodiscard]] usize size_bytes() const noexcept;

        // iterators
        [[nodiscard]] utf8::iterator begin() const noexcept;
        [[nodiscard]] utf8::iterator end() const noexcept;
        [[nodiscard]] utf8::iterator cbegin() const noexcept;
        [[nodiscard]] utf8::iterator cend() const noexcept;
        [[nodiscard]] utf8::reverse_iterator rbegin() const noexcept;
        [[nodiscard]] utf8::reverse_iterator rend() const noexcept;
        [[nodiscard]] utf8::reverse_iterator crbegin() const noexcept;
        [[nodiscard]] utf8::reverse_iterator crend() const noexcept;

        // [[nodiscard]] const code_point* code_points() const;
        [[nodiscard]] std::basic_string_view<utf8::code_unit> code_units() const;
        [[nodiscard]] const char* c_str() const;

        // code_point code_point_at(usize i) const;

        string_view view(utf8::iterator first, utf8::iterator last) const;
        string_view view_from(utf8::iterator first) const;
        string_view view_until(utf8::iterator last) const;

        // string sub(utf8::iterator first, utf8::iterator last) const;
        // string sub(utf8::iterator first, usize count) const;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] usize size() const noexcept(!Envy::debug);
        [[nodiscard]] usize capacity() const noexcept;

        void clear() noexcept;
        void reserve(usize bytes);

        void append(const char* cstr);
        void append(Envy::string_view str);
        void append(utf8::code_point cp);
        void append(char c);

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

        // [[nodiscard]] bool contains(string_view sv) const;
        // [[nodiscard]] bool contains(char c) const;
        // [[nodiscard]] bool contains(code_point cp) const;

        // [[nodiscard]] bool contains_any(string_view sv) const;
        // [[nodiscard]] bool contains_all(string_view sv) const;

        // template <typename T>
        // T parse_as()
        // {
        //     // ...
        // }

        [[nodiscard]] bool operator==(const Envy::string&) const noexcept;

    private:

        [[nodiscard]] usize new_capacity(usize required_size) noexcept;
        void adjust_buffer(usize required_size);
    };


    static_assert(std::ranges::range<string>);
    static_assert(std::ranges::bidirectional_range<string>);


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