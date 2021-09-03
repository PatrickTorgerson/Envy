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

    // TODO: global log file
    // TODO: source_location macros resolved everywhere
    // TODO: macro strip functions
    // TODO: tab stops?
    // TODO: cleanup
    // TODO: documentation
    // TODO: noexcept logging
    // TODO: Every log destination has its own indent count


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Log State ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        // -- preamble

        std::string preamble {"{DGRY}[{datetime:%T}] {file} LN{line} | {severity_color}{severity}{DGRY} : {WHT}"};

        i32 func_width            {15};
        i32 file_width            {15};
        i32 line_width            {4};
        i32 col_width             {4};
        i32 datetime_width        {16};
        i32 logger_width          {10};
        i32 severity_width        {7};
        i32 severity_short_width  {3};

        bool resolving_preamble {false};

        constexpr i32 align_left   {0};
        constexpr i32 align_center {1};
        constexpr i32 align_right  {2};

        // -- indent

        i32 indent_count {};
        std::string indent_str {"|   "};

        // -- message state

        log_message_source  msg_source;
        log_severity        msg_severity;
        Envy::string        msg_logger;

        // -- macros

        macro_map colors_nop;
        macro_map log_macros;

        // -- mutex

        std::mutex console_mutex;
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::logger ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    logger::logger(std::string name) noexcept :
        name {std::move(name)}
    { }


    //**********************************************************************
    logger::logger(std::string name, std::string log_file) noexcept :
        name     {std::move(name)},
        logfile  {std::move(log_file)}
    { }


    //**********************************************************************
    logger::logger(std::string name, std::string log_file, bool console) noexcept :
        name             { std::move(name) },
        logfile          { std::move(log_file) },
        console_logging  { console }
    { }


    //**********************************************************************
    log_message logger::message(log_severity severity, std::string_view fmt, std::source_location loc)
    {
        update_log_state(name, severity, loc);

        return log_message
        {
            *this,
            severity,
            std::move(loc),
            Envy::expand_macros(fmt, log_macros)
        };
    }


    //**********************************************************************
    void logger::assert(bool test, std::string_view msg, std::source_location loc)
    {
        if(!test)
        {
            update_log_state(name, log_severity::assert, loc);
            Envy::raw_log(logfile, console_logging, msg);
            throw Envy::assertion(msg, loc);
        }
    }


    //**********************************************************************
    void logger::debug_assert(bool test, std::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    {
        ENVY_DEBUG_CALL(assert(test,msg,loc));
    }


    //**********************************************************************
    log_message logger::error(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::error, fmt, loc);
    }


    //**********************************************************************
    log_message logger::warning(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::warning, fmt, loc);
    }


    //**********************************************************************
    log_message logger::note(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::note, fmt, loc);
    }


    //**********************************************************************
    log_message logger::info(std::string_view fmt, std::source_location loc)
    {
        return message(log_severity::info, fmt, loc);
    }


    //**********************************************************************
    void logger::seperator()
    {
        // TODO: make 'sep' global, add set_sperator()
        constexpr char sep[] {"======================================================================================================================================================\n"};

        if(console_logging)
        {
            std::scoped_lock l {console_mutex};
            print(log_macros.expand("DGRY"));
            print(sep);
            print(log_macros.expand("WHT"));
        }

        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::app);

            fs << sep;

            fs.close();
        }
    }


    //**********************************************************************
    bool logger::logs_to_console() const noexcept
    { return console_logging; }


    //**********************************************************************
    std::string logger::get_file() const noexcept
    { return logfile; }


    //**********************************************************************
    void logger::enable_console_logging(bool b) noexcept
    { console_logging = b; }


    //**********************************************************************
    void logger::set_file(std::string file) noexcept
    { logfile = std::move(file); }


    //**********************************************************************
    void logger::clear_file()
    {
        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::trunc);
            fs.close();
        }
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::scope_logger ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    std::string_view logger::get_name()
    { return name; }


    //**********************************************************************
    scope_logger::scope_logger(std::string msg, logger& l, std::source_location loc) :
        log {l}
    {
        update_log_state(log.get_name(), log_severity::scope, loc);
        raw_log(log.get_file(), log.logs_to_console(), expand_macros("{{ {DGRY}" + msg, log_macros));
        indent_log();
        t = std::chrono::high_resolution_clock::now();
    }


    //**********************************************************************
    scope_logger::~scope_logger()
    {
        std::chrono::duration<f64> delta { std::chrono::high_resolution_clock::now() - t };
        unindent_log();
        update_log_state(log.get_name(), log_severity::scope, {});
        raw_log(log.get_file(), log.logs_to_console(), expand_macros("} {DGRY}" + std::format("{}", delta), log_macros));
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Forwards ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    std::string indent_string();
    std::string process_message(std::string_view msg);

    Envy::string func_macro(Envy::string_view param);
    Envy::string file_macro(Envy::string_view param);
    Envy::string line_macro(Envy::string_view param);
    Envy::string col_macro(Envy::string_view param);
    Envy::string datetime_macro(Envy::string_view param);
    Envy::string severity_macro(Envy::string_view param);
    Envy::string severity_short_macro(Envy::string_view param);
    Envy::string severity_color_macro(Envy::string_view param);
    Envy::string logger_name_macro(Envy::string_view param);

    Envy::string clamp(Envy::string_view s, i32 width, i32 align, char fill);


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Global log functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void init_logging()
    {
        // -- Color macros

        log_macros.add("BLK", "\x1b[30m");
        log_macros.add("RED", "\x1b[31m");
        log_macros.add("GRN", "\x1b[32m");
        log_macros.add("YEL", "\x1b[33m");
        log_macros.add("BLU", "\x1b[34m");
        log_macros.add("MAG", "\x1b[35m");
        log_macros.add("CYN", "\x1b[36m");
        log_macros.add("LGRY", "\x1b[37m");

        log_macros.add("DGRY", "\x1b[90m");
        log_macros.add("LRED", "\x1b[91m");
        log_macros.add("LGRN", "\x1b[92m");
        log_macros.add("LYEL", "\x1b[93m");
        log_macros.add("LBLU", "\x1b[94m");
        log_macros.add("LMAG", "\x1b[95m");
        log_macros.add("LCYN", "\x1b[96m");
        log_macros.add("WHT", "\x1b[97m");

        log_macros.add("DEF", "\x1b[39m");
        log_macros.add("CLR", "\x1b[0m");

        // -- color stripping macros

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

        // -- source location macros

        log_macros.add("file", file_macro);
        log_macros.add("line", line_macro);
        log_macros.add("col" , col_macro);
        log_macros.add("func", func_macro);

        // -- message macros

        log_macros.add("severity_short",  severity_short_macro);
        log_macros.add("severity",        severity_macro);
        log_macros.add("severity_color",  severity_color_macro);
        log_macros.add("logger",          logger_name_macro);

        // -- datetime

        log_macros.add("datetime", datetime_macro);
    }


    //**********************************************************************
    void indent_log()
    { ++indent_count; }


    //**********************************************************************
    void unindent_log()
    { --indent_count; }


    //**********************************************************************
    log_message error(std::string fmt, std::source_location loc)
    { return log.message(log_severity::error, std::move(fmt), loc); }


    //**********************************************************************
    log_message warning(std::string fmt, std::source_location loc)
    { return log.message(log_severity::warning, std::move(fmt), loc); }


    //**********************************************************************
    log_message note(std::string fmt, std::source_location loc)
    { return log.message(log_severity::note, std::move(fmt), loc); }


    //**********************************************************************
    log_message info(std::string fmt, std::source_location loc)
    { return log.message(log_severity::info, std::move(fmt), loc); }


    //**********************************************************************
    void assert(bool test, std::string_view msg, std::source_location loc)
    { log.assert(test,msg,loc); }


    //**********************************************************************
    void debug_assert(bool test, std::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    { log.debug_assert(test,msg,loc); }


    //**********************************************************************
    void set_preamble_pattern(std::string_view pattern)
    {
        preamble = std::move(preamble);
    }


    //**********************************************************************
    void update_log_state(std::string_view logger_name, log_severity severity, log_message_source loc)
    {
        msg_source = loc;
        msg_severity = severity;

        // TODO: Envy::string copy asign from std::strin and std::string_view
        msg_logger = Envy::string(logger_name);
    }


    //**********************************************************************
    void raw_log(std::string_view logger_file, bool log_to_console, std::string_view msg)
    {
        // -- process message

        std::string logmsg { process_message(msg) };

        // -- log message

        if(log_to_console)
        {
            std::scoped_lock l {console_mutex};
            print(logmsg);
        }

        if(!logger_file.empty())
        {
            std::ofstream fs;
            fs.open(logger_file, std::ios::app);

            // TODO: skip writing ansi escape sequences
            fs << logmsg;

            fs.close();
        }
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    std::string process_message(std::string_view msg)
    {

        // u64 preamble_size      { expand_macros(preamble, colors_nop, log_macros).size() };
        // u64 note_preamble_size { expand_local_macros(note_preamble, colors_nop, log_macros)->size() };

        // // number of chars we have to pad the note premble with to align with the rest of the log messages
        // u64 fill_count { preamble_size - note_preamble_size };

        // // pads note premble with gray dots, adds necessary indents
        // std::string resolved_note_preamble { resolve_note_preamble(fill_count) };

        resolving_preamble = true;
        std::string resolved_preamble { Envy::expand_macros(preamble, log_macros) };
        resolving_preamble = false;

        std::string logmsg { };
        logmsg.reserve( msg.size() * 2u );

        logmsg += resolved_preamble;

        logmsg += Envy::expand_macros(indent_string(), log_macros);

        if(msg_severity == log_severity::note)
        {
            logmsg += "{DGRY}";
        }

        msg_severity = log_severity::note;
        resolving_preamble = true;
        indent_log();

        // every newline is a note, so add the note preamble
        logmsg += Envy::replace(msg, "\n", "\n" + (std::string) Envy::expand_macros(preamble + indent_string() + "{DGRY}", log_macros)) + "\x1b[0m\n";

        unindent_log();
        resolving_preamble = false;

        return logmsg;
    }


    //**********************************************************************
    std::string indent_string()
    {
        if(indent_count <= 0)
        { return ""; }

        std::string result;
        result.reserve(indent_count * indent_str.size());

        result += "{DGRY}";

        for(int i {}; i < indent_count; ++i)
        { result += indent_str; }

        result += "{WHT}";

        return result;
    }


    //**********************************************************************
    Envy::string func_macro(Envy::string_view param)
    {
        Envy::string r { std::format( build_fmt_str(param) , msg_source.func ) };

        if(resolving_preamble)
        { return clamp(r, func_width, align_right, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string file_macro(Envy::string_view param)
    {
        Envy::string r { std::format( build_fmt_str(param) , std::filesystem::path {msg_source.file} .filename().string() ) };

        if(resolving_preamble)
        { return clamp(r, file_width, align_right, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string line_macro(Envy::string_view param)
    {
        Envy::string r { std::format( build_fmt_str(param) , msg_source.line ) };

        if(resolving_preamble)
        { return clamp(r, line_width, align_right, '0'); }
        else return r;
    }


    //**********************************************************************
    Envy::string col_macro(Envy::string_view param)
    {
        Envy::string r { std::format( build_fmt_str(param) , msg_source.col ) };

        if(resolving_preamble)
        { return clamp(r, col_width, align_right, '0'); }
        else return r;
    }


    //**********************************************************************
    Envy::string datetime_macro(Envy::string_view param)
    {
        std::chrono::zoned_time time {std::chrono::current_zone(), std::chrono::system_clock::now()};

        Envy::string r { std::format( build_fmt_str(param) , time ) };

        if(resolving_preamble)
        { return clamp(r, datetime_width, align_left, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string severity_macro(Envy::string_view param)
    {
        constexpr const char* severities[]
        {"scope", "assert", "error", "warning", "note", "info"};

        Envy::string r { std::format( build_fmt_str(param) , severities[static_cast<i32>(msg_severity)] ) };

        if(resolving_preamble)
        { return clamp(r, severity_width, align_right, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string severity_short_macro(Envy::string_view param)
    {
        constexpr const char* severities_short[]
        {"scp", "asr", "err", "wrn", "nte", "inf"};

        Envy::string r { std::format( build_fmt_str(param) , severities_short[static_cast<i32>(msg_severity)] ) };

        if(resolving_preamble)
        { return clamp(r, severity_short_width, align_right, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string severity_color_macro(Envy::string_view param)
    {
        constexpr const char* severity_colors[]
        {"{DGRY}", "{RED}", "{LRED}", "{LYEL}", "{LMAG}", "{LCYN}"};

        return severity_colors[static_cast<i32>(msg_severity)];
    }


    //**********************************************************************
    Envy::string logger_name_macro(Envy::string_view param)
    {
        Envy::string r { std::format( build_fmt_str(param) , (std::string) msg_logger ) };

        if(resolving_preamble)
        { return clamp(r, logger_width, align_right, ' '); }
        else return r;
    }


    //**********************************************************************
    Envy::string clamp(Envy::string_view s, i32 width, i32 align, char fill)
    {
        if(width <= 2)
        { return Envy::string { (usize) width, '.' }; }

        if(s.empty())
        { return Envy::string { (usize) width, fill }; }

        if(s.size() == width)
        { return s; }

        Envy::string r { Envy::string::reserve_tag, (usize) width };

        constexpr char cut[] { ".." };


        if( width > s.size() )
        {
            // fill to width

            f64 a { align / 2.0 };
            i32 f { width - (i32) s.size() };

            i32 fill_right { (i32) std::floor( f * (1.0-a)) };
            i32 fill_left  { (i32)  std::ceil( f * (  a  )) };

            r += Envy::string( fill_left, fill );
            r += s;
            r += Envy::string( fill_right, fill );

            return r;
        }
        else
        {
            // cut to width

            i32 c { (i32) s.size() - (width - ((i32) std::size(cut)-1)) };

            if(align == align_right)
            {
                auto it {s.begin()};

                while(--c >= 0) ++it;

                r += cut;
                r += s.view_from(it);

                return r;
            }
            else
            {
                auto it {s.end()};

                while(--c >= 0) --it;

                r += s.view_until(it);
                r += cut;

                return r;
            }
        }


    }
}