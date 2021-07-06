#include <log.hpp>

#include <string>
#include <format>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <chrono>

#include <string.hpp>
#include <exception.hpp>

namespace Envy
{
    namespace
    {
        std::string preamble {"{BBLK}[{datetime}]  {file: >15}  LN{line:0>4} | {lvlcol}{level: >7}{BBLK} : {BWHT}"};
    }


    void set_preamble_pattern(std::string_view pattern)
    {
        preamble = std::move(preamble);
    }


    void print_preamble(logger::level lvl, std::source_location l)
    {
        macro_map preamble_macros;

        constexpr const char* lvls[]
        {"err", "wrn", "nte", "inf"};
        constexpr const char* levels[]
        {"error", "warning", "note", "info"};
        constexpr const char* lvlcols[]
        {"{BRED}", "{BYEL}", "{BMAG}", "{BCYN}"};

        std::filesystem::path file {l.file_name()};

        std::chrono::zoned_time time {std::chrono::current_zone(), std::chrono::system_clock::now()};

        macro(preamble_macros, "file", file.filename());
        macro(preamble_macros, "line", l.line());
        macro(preamble_macros, "col" , l.column());
        macro(preamble_macros, "func", l.function_name());
        macro(preamble_macros, "lvl", lvls[static_cast<i32>(lvl)]);
        macro(preamble_macros, "level", levels[static_cast<i32>(lvl)]);
        macro(preamble_macros, "lvlcol", lvlcols[static_cast<i32>(lvl)]);
        macro(preamble_macros, "datetime", std::format("{:%m-%d-%Y %T}",time));

        print( resolve(preamble, preamble_macros) );
    }


    // logger


    logger::logger(std::string log_file)
        : logfile {log_file}
    {}


    void logger::log(level lvl, const std::string& str, std::source_location loc)
    {
        print_preamble(lvl, loc);
        printl( resolve(str) );
        print(resolve("{CLR}"));
    }


    bool logger::logs_to_console() const noexcept
    { return console_logging; }


    std::string logger::file() const noexcept
    { return logfile; }


    void logger::enable_console_logging(bool b) noexcept
    { console_logging = b; }


    void logger::set_file(std::string file)
    { logfile = std::move(file); }


    void assert(bool test, std::string_view msg, std::source_location loc)
    {
        if(!test)
        {
            Envy::exception e(msg,loc);
            log.error(e.what());
            throw e;
        }
    }

    void debug_assert(bool test, std::string_view msg, std::source_location loc)
    {
        ENVY_DEBUG_CALL(assert(test,msg,loc));
    }


}