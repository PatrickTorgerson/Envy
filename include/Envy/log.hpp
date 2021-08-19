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

#include <format>
#include <source_location>
#include <filesystem>
#include <chrono>

// TODO: noexcept logging

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


    enum class log_severity : u8
    {
        scope,
        assert,
        error,
        warning,
        note,
        info
    };


    void raw_log(std::string_view logger_name, std::string_view logger_file, bool log_to_console, log_severity severity, std::string_view msg, const char* file, u32 line, u32 col, const char* func);


    class log_message;


    class logger
    {

        std::string name {};
        std::string logfile {};
        bool console_logging {true};

    public:

        explicit logger(std::string name) noexcept;
        logger(std::string name, std::string log_file) noexcept;
        logger(std::string name, std::string log_file, bool console) noexcept;

        log_message message(log_severity severity, std::string_view fmt, std::source_location loc = std::source_location::current());

        log_message error(std::string_view fmt, std::source_location loc = std::source_location::current());
        log_message warning(std::string_view fmt, std::source_location loc = std::source_location::current());
        log_message note(std::string_view fmt, std::source_location loc = std::source_location::current());
        log_message info(std::string_view fmt, std::source_location loc = std::source_location::current());

        void assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());
        void debug_assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);

        bool logs_to_console() const noexcept;
        void enable_console_logging(bool b) noexcept;

        void seperator();

        std::string get_file() const noexcept;
        void set_file(std::string file);
        void clear_file();

        std::string_view get_name();
    };


    void indent_log();
    void unindent_log();
    void log_seperator();

    class log_message
    {
    public:

        logger& log;
        log_severity severity;
        std::source_location loc;
        std::string fmt;

        log_message(logger& log, log_severity severity, std::source_location loc, std::string fmt) noexcept :
            log {log},
            severity {severity},
            loc {std::move(loc)},
            fmt {std::move(fmt)}
        { }

        ~log_message()
        {
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), severity, fmt, loc.file_name(), loc.line(), loc.column(), loc.function_name());
        }

        log_message(const log_message&) = delete;
        log_message(log_message&&) = delete;
        log_message& operator=(const log_message&) = delete;
        log_message& operator=(log_message&&) = delete;

        template <convertable_to_string ... Ts>
        log_message& operator()(Ts&& ... args)
        {
            fmt = std::format( fmt, std::forward<Ts>(args)... );
            return *this;
            //raw_log(log.get_name(), log.get_file(), log.logs_to_console(), severity, std::format( fmt, std::forward<Ts>(args)... ), loc.file_name(), loc.line(), loc.column(), loc.function_name());
        }

        template <convertable_to_string ... Ts>
        log_message& note(const std::string& fmtstr, Ts&& ... args)
        {
            fmt += std::format("\n"+fmtstr, std::forward<Ts>(args)...);
            return *this;
        }
    };


    inline logger log {"Envy"};


    class scope_logger
    {
        logger& log;
        std::chrono::high_resolution_clock::time_point t;
    public:

        explicit scope_logger(std::string msg, logger& l = Envy::log, std::source_location loc = std::source_location::current()) :
            log {l}
        {
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), log_severity::scope, "{ {BBLK}" + msg, loc.file_name(), loc.line(), loc.column(), loc.function_name());
            indent_log();
            t = std::chrono::high_resolution_clock::now();
        }

        ~scope_logger()
        {
            std::chrono::duration<f64> delta { std::chrono::high_resolution_clock::now() - t };
            unindent_log();
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), log_severity::scope, "} " + std::format("{{BBLK}}{}{{BWHT}}", delta), "", 0, 0, "");
        }
    };


    void init_logging();

    log_message error(std::string fmt, std::source_location loc = std::source_location::current());
    log_message warning(std::string fmt, std::source_location loc = std::source_location::current());
    log_message note(std::string fmt, std::source_location loc = std::source_location::current());
    log_message info(std::string fmt, std::source_location loc = std::source_location::current());

    void set_preamble_pattern(std::string_view pattern);
    void print_preamble(const std::string& name, log_severity lvl, std::source_location loc = std::source_location::current());

    void assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());
    void debug_assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);

}