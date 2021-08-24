#include <log.hpp>

#include <string>
#include <format>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>
#include <mutex>

#include <string.hpp>
#include <macro.hpp>
#include <exception.hpp>

namespace Envy
{
    namespace
    {
        std::string preamble {"{BBLK}[{datetime}] {file: >20} LN{line:0>4} | {name: >10} {severity_color}{severity}{BBLK} : {BWHT}"};
        std::string note_preamble {" {BMAG}note{BBLK} : "};

        macro_map colors;
        macro_map colors_nop;

        i32 indent_count {};
        std::string indent_str {"|   "};

        std::mutex console_mutex;
    }


    // TODO: global log file
    // TODO: source_location macros resolved everywhere
    // TODO: macro strip functions
    // TODO: tab stops?
    // TODO: cleanup
    // TODO: documentation


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


    void indent_log()
    { ++indent_count; }


    void unindent_log()
    { --indent_count; }


    std::string generate_indent(bool forconsole = false)
    {
        if(indent_count <= 0)
        { return ""; }

        std::string result;
        result.reserve(indent_count * indent_str.size());

        if(forconsole)
        { result += "{BBLK}"; }

        for(int i {}; i < indent_count; ++i)
        { result += indent_str; }

        if(forconsole)
        { result += "{BWHT}"; }

        return result;
    }


    std::string resolve_preamble(std::string_view name, log_severity severity, const char* file, u32 line, u32 col, const char* func)
    {
        // TODO: make global, build in init_logging()
        macro_map preamble_macros;

        constexpr const char* severities_short[]
        {"scp", "ast", "err", "wrn", "nte", "inf"};
        constexpr const char* severities[]
        {"  scope", " assert", "  error", "warning", "   note", "   info"};
        constexpr const char* severity_colors[]
        {"{BBLK}", "{RED}", "{BRED}", "{BYEL}", "{BMAG}", "{BCYN}"};

        std::filesystem::path path {file};

        std::chrono::zoned_time time {std::chrono::current_zone(), std::chrono::system_clock::now()};

        macro(preamble_macros, "file",            path.filename());
        macro(preamble_macros, "line",            line);
        macro(preamble_macros, "col" ,            col);
        macro(preamble_macros, "func",            func);
        macro(preamble_macros, "severity_short",  severities_short[static_cast<i32>(severity)]);
        macro(preamble_macros, "severity",        severities[static_cast<i32>(severity)]);
        macro(preamble_macros, "severity_color",  severity_colors[static_cast<i32>(severity)]);
        macro(preamble_macros, "datetime",        std::format("{:%m-%d-%Y %T}",time));
        macro(preamble_macros, "name",            name);

        return resolve(preamble, preamble_macros);
    }


    std::string resolve_note_preamble(u64 fill_count)
    {
        std::string result {"\n{BBLK}"};

        // pads note premble with dashes
        result += std::format("{:->{}}", "", fill_count);

        result += note_preamble;

        result += "{BBLK}";
        result += generate_indent(false) + indent_str;

        return result;
    }


    void set_preamble_pattern(std::string_view pattern)
    {
        preamble = std::move(preamble);
    }


    void raw_log(std::string_view logger_name, std::string_view logger_file, bool log_to_console, log_severity severity, std::string_view msg, const char* file, u32 line, u32 col, const char* func)
    {
        // resolve_preamble() leaves color macros ( {BBLK} ... )
        std::string resolved_preamble { resolve_preamble(logger_name, severity, file, line, col, func) };

        // strip color macros by resolving with 'colors_nop'
        u64 preamble_size      { resolve_local(resolved_preamble, colors_nop).size() };
        u64 note_preamble_size { resolve_local(note_preamble,     colors_nop).size() };

        // number of chars we have to bad the note premble with to align with the rest of the log messages
        u64 fill_count { preamble_size - note_preamble_size };

        // pads note premble with gray dots, adds necessary indents
        std::string resolved_note_preamble { resolve_note_preamble(fill_count) };

        // every newline is a note, so add the note preamble
        std::string msgstr {};

        if(severity == log_severity::note)
        { indent_log(); msgstr += "{BBLK}"; }

        msgstr += Envy::replace(msg, "\n", resolved_note_preamble);

        if(log_to_console)
        {
            std::scoped_lock l {console_mutex};

            // resolve colors and print to console
            print(resolve_local(resolved_preamble,colors));
            print(resolve_local(generate_indent(true),colors));
            print(resolve_local(msgstr,colors));
            printl("\x1b[0m");
        }

        if(!logger_file.empty())
        {
            std::ofstream fs;
            fs.open(logger_file, std::ios::app);

            // Note: colors_nop expands color macros to empty strings to avoid outputing ansi-escape sequences to the log file
            fs << resolve_local(resolved_preamble,colors_nop);
            fs << generate_indent();
            fs << resolve_local(msgstr,colors_nop);
            fs << '\n';

            fs.close();
        }

        if(severity == log_severity::note)
        { unindent_log(); }
    }


    // ==== logger


    logger::logger(std::string name) noexcept :
        name {std::move(name)}
    { }


    logger::logger(std::string name, std::string log_file) noexcept :
        name     {std::move(name)},
        logfile  {std::move(log_file)}
    { }


    logger::logger(std::string name, std::string log_file, bool console) noexcept :
        name             {std::move(name)},
        logfile          {std::move(log_file)},
        console_logging  {console}
    { }


    log_message logger::message(log_severity severity, std::string_view fmt, std::source_location loc)
    {
        return log_message
        {
            *this,
            severity,
            std::move(loc),
            Envy::resolve(fmt, colors)
        };
    }


    void logger::assert(bool test, std::string_view msg, std::source_location loc)
    {
        if(!test)
        {
            Envy::raw_log(name, logfile, console_logging, Envy::log_severity::assert, msg, loc.file_name(), loc.line(), loc.column(), loc.function_name());
            throw Envy::assertion(msg, loc);
        }
    }


    void logger::debug_assert(bool test, std::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    {
        ENVY_DEBUG_CALL(assert(test,msg,loc));
    }


    log_message logger::error(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::error, fmt, loc);
    }


    log_message logger::warning(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::warning, fmt, loc);
    }


    log_message logger::note(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::note, fmt, loc);
    }


    log_message logger::info(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::info, fmt, loc);
    }


    void logger::seperator()
    {
        constexpr char sep[] {"======================================================================================================================================================"};

        if(console_logging)
        {
            std::scoped_lock l {console_mutex};
            print(resolve("{BBLK}",colors));
            print(sep);
            print(resolve("{BWHT}",colors));
            print("\n");
        }

        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::app);

            // Note: colors_nop expands color macros to empty strings to avoid outputing ansi-escape sequences to the log file
            fs << sep;
            fs << '\n';

            fs.close();
        }
    }


    bool logger::logs_to_console() const noexcept
    { return console_logging; }


    std::string logger::get_file() const noexcept
    { return logfile; }


    void logger::enable_console_logging(bool b) noexcept
    { console_logging = b; }


    void logger::set_file(std::string file) noexcept
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


    std::string_view logger::get_name()
    { return name; }


    // ==== global log functions


    log_message error(std::string fmt, std::source_location loc)
    { return log.message(log_severity::error, std::move(fmt), loc); }


    log_message warning(std::string fmt, std::source_location loc)
    { return log.message(log_severity::warning, std::move(fmt), loc); }


    log_message note(std::string fmt, std::source_location loc)
    { return log.message(log_severity::note, std::move(fmt), loc); }


    log_message info(std::string fmt, std::source_location loc)
    { return log.message(log_severity::info, std::move(fmt), loc); }


    void assert(bool test, std::string_view msg, std::source_location loc)
    { log.assert(test,msg,loc); }

    void debug_assert(bool test, std::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    { log.debug_assert(test,msg,loc); }


}