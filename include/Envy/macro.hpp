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
#include "string.hpp"

#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>

namespace Envy
{
    // TODO: wrapper type for macro_map
    // TODO: rename function 'macro' so as not to collide with wrapper type
    using macro_t = std::function<std::string(void)>;
    using macro_map = std::unordered_map<std::string,macro_t>;

    void macro(std::string_view name, macro_t m);
    void macro(macro_map& map, std::string_view name, macro_t m);

    inline void macro(std::string_view name, std::string(*mfn)(void))
    { macro(name, macro_t(mfn)); }
    inline void macro(macro_map& map, std::string_view name, std::string(*mfn)(void))
    { macro(map, name, macro_t(mfn)); }

    template <convertable_to_string T>
    void macro(std::string_view name, T&& v)
    { macro(name, [s=to_string(std::forward<T>(v))](){ return s; }); }

    template <convertable_to_string T>
    void macro(macro_map& map, std::string_view name, T&& v)
    { macro(map, name, [s=to_string(std::forward<T>(v))](){ return s; }); }

    // TODO: resolve takes a variable number of macro_maps
    [[nodiscard]] std::string resolve_local(std::string_view s, const macro_map& map);
    [[nodiscard]] std::string resolve(std::string_view);
    [[nodiscard]] std::string resolve(std::string_view, const macro_map& additional);
}