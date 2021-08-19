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
#include "log.hpp"

#include <source_location>
#include <string>
#include <string_view>
#include <concepts>
#include <vector>

namespace Envy
{

    class test_case
    {
        friend class test_state;

        std::string name;
        std::source_location loc;

        std::vector<std::string> failure_msgs;
        std::vector<std::source_location> failure_locs;

    public:

        test_case() = default;
        test_case(std::string name, std::source_location loc = std::source_location::current());

        void require(bool condition, std::string_view msg = "", std::source_location loc = std::source_location::current());
    };


    class test_state
    {
        u32 total   {0};
        u32 passed  {0};

        std::string current_test {};
        bool current_status {true};

        Envy::logger log {"Test"};

        bool verbose_flag {false};

        std::chrono::high_resolution_clock::time_point time;

    public:

        struct verbose_t {};
        static constexpr verbose_t verbose {};

        explicit test_state(std::source_location loc = std::source_location::current());
        explicit test_state(verbose_t, std::source_location loc = std::source_location::current());

        void start(std::source_location loc = std::source_location::current());
        void start(std::string name, std::source_location loc = std::source_location::current());

        void add_case(bool condition, std::string_view name, std::string_view msg = "", std::source_location loc = std::source_location::current());

        void add_case(test_case c, std::source_location loc = std::source_location::current());

        void submit(std::source_location loc = std::source_location::current());

        bool report(std::source_location loc = std::source_location::current());

        void set_verbose(bool v) noexcept;

        [[nodiscard]] Envy::logger& get_logger() noexcept;
    };

}