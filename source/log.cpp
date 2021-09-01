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
        std::string preamble {"{DGRY}[{datetime}] {file: >20} LN{line:0>4} | {name: >10} {severity_color}{severity}{DGRY} : {WHT}"};
        std::string note_preamble {" {MAG}note{DGRY} : "};

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
        colors.add("BLK", "\x1b[30m");
        colors.add("RED", "\x1b[31m");
        colors.add("GRN", "\x1b[32m");
        colors.add("YEL", "\x1b[33m");
        colors.add("BLU", "\x1b[34m");
        colors.add("MAG", "\x1b[35m");
        colors.add("CYN", "\x1b[36m");
        colors.add("LGRY", "\x1b[37m");

        colors.add("DGRY", "\x1b[90m");
        colors.add("LRED", "\x1b[91m");
        colors.add("LGRN", "\x1b[92m");
        colors.add("LYEL", "\x1b[93m");
        colors.add("LBLU", "\x1b[94m");
        colors.add("LMAG", "\x1b[95m");
        colors.add("LCYN", "\x1b[96m");
        colors.add("WHT", "\x1b[97m");

        colors.add("DEF", "\x1b[39m");
        colors.add("CLR", "\x1b[0m");

        colors_nop.add("BLK", "");
        colors_nop.add("RED", "");
        colors_nop.add("GRN", "");
        colors_nop.add("YEL", "");
        colors_nop.add("BLU", "");
        colors_nop.add("MAG", "");
        colors_nop.add("CYN", "");
        colors_nop.add("LGYR", "");

        colors_nop.add("DGRY", "");
        colors_nop.add("LRED", "");
        colors_nop.add("LGRN", "");
        colors_nop.add("LYEL", "");
        colors_nop.add("LBLU", "");
        colors_nop.add("LMAG", "");
        colors_nop.add("LCYN", "");
        colors_nop.add("WHT", "");

        colors_nop.add("CLR", "");
        colors_nop.add("DEF", "");
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
        { result += "{DGRY}"; }

        for(int i {}; i < indent_count; ++i)
        { result += indent_str; }

        if(forconsole)
        { result += "{WHT}"; }

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
        {"{DGRY}", "{LRED}", "{RED}", "{LYEL}", "{LMAG}", "{LCYN}"};

        std::filesystem::path path {file};

        std::chrono::zoned_time time {std::chrono::current_zone(), std::chrono::system_clock::now()};

        preamble_macros.add("file",            Envy::to_string(path.filename()));
        preamble_macros.add("line",            line);
        preamble_macros.add("col" ,            col);
        preamble_macros.add("func",            func);
        preamble_macros.add("severity_short",  severities_short[static_cast<i32>(severity)]);
        preamble_macros.add("severity",        severities[static_cast<i32>(severity)]);
        preamble_macros.add("severity_color",  severity_colors[static_cast<i32>(severity)]);
        preamble_macros.add("datetime",        std::format("{:%m-%d-%Y %T}",time));
        preamble_macros.add("name",            name);

        return expand_macros(preamble, preamble_macros);
    }


    std::string resolve_note_preamble(u64 fill_count)
    {
        std::string result {"\n{DGRY}"};

        // pads note premble with dashes
        result += std::format("{:->{}}", "", fill_count);

        result += note_preamble;

        result += "{DGRY}";
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

        // strip color macros by expanding with 'colors_nop'
        u64 preamble_size      { expand_local_macros(resolved_preamble, colors_nop)->size() };
        u64 note_preamble_size { expand_local_macros(note_preamble,     colors_nop)->size() };

        // number of chars we have to bad the note premble with to align with the rest of the log messages
        u64 fill_count { preamble_size - note_preamble_size };

        // pads note premble with gray dots, adds necessary indents
        std::string resolved_note_preamble { resolve_note_preamble(fill_count) };

        // every newline is a note, so add the note preamble
        std::string msgstr {};

        if(severity == log_severity::note)
        { indent_log(); msgstr += "{DGRY}"; }

        msgstr += Envy::replace(msg, "\n", resolved_note_preamble);

        if(log_to_console)
        {
            std::scoped_lock l {console_mutex};

            // resolve colors and print to console
            print(expand_local_macros(resolved_preamble,colors));
            print(expand_local_macros(generate_indent(true),colors));
            print(expand_macros(msgstr,colors));
            printl("\x1b[0m");
        }

        if(!logger_file.empty())
        {
            std::ofstream fs;
            fs.open(logger_file, std::ios::app);

            // Note: colors_nop expands color macros to empty strings to avoid outputing ansi-escape sequences to the log file
            fs << (std::string&&) expand_local_macros(resolved_preamble,colors_nop);
            fs << (std::string&&) generate_indent();
            fs << (std::string&&) expand_macros(msgstr,colors_nop);
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
            Envy::expand_macros(fmt, colors)
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
        // TODO: make 'sep' global, add set_sperator()
        constexpr char sep[] {"======================================================================================================================================================\n"};

        if(console_logging)
        {
            std::scoped_lock l {console_mutex};
            print(colors.expand("DGRY"));
            print(sep);
            print(colors.expand("WHT"));
        }

        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::app);

            fs << sep;

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