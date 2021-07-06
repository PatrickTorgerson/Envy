#include <log.hpp>

#include <string>
#include <format>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>

#include <string.hpp>
#include <exception.hpp>

namespace Envy
{
    namespace
    {
        std::string preamble {"{BBLK}[{datetime}] {file: >20} LN{line:0>4} | {lvlcol}{level: >7}{BBLK} : {BWHT}{name: >10}, "};
        macro_map colors;
        macro_map colors_nop;
    }


    void init_logging()
    {
        macro(colors, "BLK", "\x1b[30m");
        macro(colors, "RED", "\x1b[31m");
        macro(colors, "GRN", "\x1b[32m");
        macro(colors, "YEL", "\x1b[33m");
        macro(colors, "BLU", "\x1b[34m");
        macro(colors, "MAG", "\x1b[35m");
        macro(colors, "CYN", "\x1b[36m");
        macro(colors, "WHT", "\x1b[37m");
        macro(colors, "BBLK", "\x1b[1;30m");
        macro(colors, "BRED", "\x1b[1;31m");
        macro(colors, "BGRN", "\x1b[1;32m");
        macro(colors, "BYEL", "\x1b[1;33m");
        macro(colors, "BBLU", "\x1b[1;34m");
        macro(colors, "BMAG", "\x1b[1;35m");
        macro(colors, "BCYN", "\x1b[1;36m");
        macro(colors, "BWHT", "\x1b[1;37m");
        macro(colors, "CLR", "\x1b[0m");

        macro(colors_nop, "BLK", "");
        macro(colors_nop, "RED", "");
        macro(colors_nop, "GRN", "");
        macro(colors_nop, "YEL", "");
        macro(colors_nop, "BLU", "");
        macro(colors_nop, "MAG", "");
        macro(colors_nop, "CYN", "");
        macro(colors_nop, "WHT", "");
        macro(colors_nop, "BBLK", "");
        macro(colors_nop, "BRED", "");
        macro(colors_nop, "BGRN", "");
        macro(colors_nop, "BYEL", "");
        macro(colors_nop, "BBLU", "");
        macro(colors_nop, "BMAG", "");
        macro(colors_nop, "BCYN", "");
        macro(colors_nop, "BWHT", "");
        macro(colors_nop, "CLR", "");
    }


    void set_preamble_pattern(std::string_view pattern)
    {
        preamble = std::move(preamble);
    }


    std::string resolve_preamble(const std::string& name, logger::level lvl, std::source_location l)
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

        macro(preamble_macros, "file",      file.filename());
        macro(preamble_macros, "line",      l.line());
        macro(preamble_macros, "col" ,      l.column());
        macro(preamble_macros, "func",      l.function_name());
        macro(preamble_macros, "lvl",       lvls[static_cast<i32>(lvl)]);
        macro(preamble_macros, "level",     levels[static_cast<i32>(lvl)]);
        macro(preamble_macros, "lvlcol",    lvlcols[static_cast<i32>(lvl)]);
        macro(preamble_macros, "datetime",  std::format("{:%m-%d-%Y %T}",time));
        macro(preamble_macros, "name",      name);

        return resolve(preamble, preamble_macros);
    }


    void print_preamble(const std::string& name, logger::level lvl, std::source_location l)
    {
        print( resolve_local(resolve_preamble(name, lvl, l), colors) );
    }


    // logger


    logger::logger(std::string name) :
        name {std::move(name)}
    { }


    logger::logger(std::string name, std::string log_file) :
        name     {std::move(name)},
        logfile  {std::move(log_file)}
    { }


    logger::logger(std::string name, std::string log_file, bool console) :
        name             {std::move(name)},
        logfile          {std::move(log_file)},
        console_logging  {console}
    { }


    void logger::log(level lvl, const std::string& str, std::source_location loc)
    {
        std::string msg {resolve(str)};
        std::string pre {resolve_preamble(name, lvl,loc)};

        if(console_logging)
        {
            print(resolve_local(pre,colors));
            print(resolve_local(msg,colors));
            printl("\x1b[0m");
        }

        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::app);

            // Note: colors_nop expands color macros to emty strings to avoid outputting ansi escape sequences to the log file
            fs << resolve_local(pre,colors_nop);
            fs << resolve_local(msg,colors_nop);
            fs << '\n';

            fs.close();
        }
    }


    bool logger::logs_to_console() const noexcept
    { return console_logging; }


    std::string logger::file() const noexcept
    { return logfile; }


    void logger::enable_console_logging(bool b) noexcept
    { console_logging = b; }


    void logger::set_file(std::string file)
    { logfile = std::move(file); }


    void logger::clear_file()
    {
        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::trunc);
            fs.close();
        }
    }

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