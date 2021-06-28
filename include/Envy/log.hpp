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

#include <source_location>
#include <filesystem>

namespace Envy
{

    template <convertable_to_string T>
    void print(T&& v)
    {
        if constexpr (std::same_as<T,bool>)
        { std::cout << std::boolalpha << v; }

        else if constexpr (stream_insertable<T>)
        { std::cout << std::forward<T>(v); }

        else
        { std::cout << Envy::to_string(std::forward<T>(v)); }
    }


    template <convertable_to_string T>
    void printl(T&& v)
    { print(std::forward<T>(v)); print('\n'); }


    class logger
    {
    public:

        enum class level
        {
            error,
            warning,
            note,
            info
        };

    private:

        std::string logfile;
        bool console_logging;

    public:

        logger() = default;
        explicit logger(std::string log_file);

        void error(std::string_view s, std::source_location l = std::source_location::current());
        void warning(std::string_view s, std::source_location l = std::source_location::current());
        void note(std::string_view s, std::source_location l = std::source_location::current());
        void info(std::string_view s, std::source_location l = std::source_location::current());

        bool logs_to_console() const noexcept;
        std::string file() const noexcept;

        void enable_console_logging(bool b) noexcept;
        void set_file(std::string file);
    };

    inline logger log;

    void set_preamble_pattern(std::string_view pattern);
    void print_preamble(logger::level lvl, std::source_location loc = std::source_location::current());

}