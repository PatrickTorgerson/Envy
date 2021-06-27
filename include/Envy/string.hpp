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

#include <unordered_map>
#include <string>
#include <string_view>
#include <iostream>
#include <functional>
#include <sstream>

namespace Envy
{

    template <typename T>
    concept numeric =
        std::floating_point<T> || std::integral<T> ||
        std::convertible_to<numeric>;


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
    std::string to_string(T&& v)
    {

        if constexpr (std::same_as<T,bool>)
        { return v ? "true" : "false"; }

        else if constexpr (std::same_as<T,char>)
        { return std::string(&v,1); }

        // TODO: unicode characters

        else if constexpr (std::convertible_to<T,std::string>)
        { return (std::string) std::forward<T>(v); }

        else if constexpr (numeric<T>)
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


    using macro_t = std::function<std::string(void)>;
    using macro_map = std::unordered_map<std::string,macro_t>;

    void macro(std::string_view name, macro_t m);
    void macro(macro_map& map, std::string_view name, macro_t m);

    inline void macro(std::string_view name, std::string(*mfn)(void))
    { macro(name, macro_t(mfn)); }
    inline void macro(macro_map& map, std::string_view name, std::string(*mfn)(void))
    { macro(map, name, macro_t(mfn)); }

    template <convertable_to_string T>
    void macro(std::string_view name, const T& v)
    { macro(name, [s=to_string(v)](){ return s; }); }

    template <convertable_to_string T>
    void macro(macro_map& map, std::string_view name, const T& v)
    { macro(map, name, [s=to_string(v)](){ return s; }); }

    std::string resolve_local(std::string_view s, const macro_map& map);
    std::string resolve(std::string_view);
    std::string resolve(std::string_view, const macro_map& additional);

}