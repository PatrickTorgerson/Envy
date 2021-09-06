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

namespace Envy::log
{

    // TODO: global log file
    // TODO: source_location macros resolved everywhere
    // TODO: macro strip functions
    // TODO: tab stops?
    // TODO: cleanup
    // TODO: documentation
    // TODO: noexcept logging
    // TODO: Every log destination has its own indent count


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        // +------------------------------------------------------------------------------------------------------------------------------------ - - - -  -  -   -
        // | Date Time                 | File            | Line | Severity | Tests
        // |---------------------------|-----------------|------|----------|-------------------------------------------------------------------- - - - -  -  -   -
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |    scope | { Init diligent
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |     info | |   Context
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |     info | |   bean pole
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |     info | |   Mock up banana
        // | 09-03-2021  15:11:35.2495 |       tests.cpp | 0086 |    scope | } 0.053s
        // | 09-03-2021  15:11:35.2495 |    graphics.cpp | 0084 |     info | Window resized
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |     note | |   window size: {640,480}
        // | 09-03-2021  15:11:35.2495 |   testbench.cpp | 0086 |     note | |   buffer size: {640,480}
        // +------------------------------------------------------------------------------------------------------------------------------------ - - - -  -  -   -

        description desc;

        i32 preamble_width;

        Envy::string border_color;
        Envy::string message_color;

        std::vector<Envy::string> column_colors;

        color severity_colors[]
        { color::dark_gray , color::red , color::light_red , color::light_yellow , color::light_magenta , color::light_cyan };

        Envy::string header;
        Envy::string header_underline;
        Envy::string note_preamble;

        // -- log state

        i32 error_count {};
        i32 warning_count {};

        // -- indent

        i32 indent_count {};

        // -- message state

        message_source  msg_source;
        severity        msg_severity;
        Envy::string    msg_logger;

        // -- macros

        macro_map colors_nop;
        macro_map log_macros;

        // -- mutex

        std::mutex console_mutex;
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Forwards ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    static [[nodiscard]] Envy::string indent_string();
    static [[nodiscard]] Envy::string process_message(Envy::string_view msg);

    static [[nodiscard]] Envy::string build_preamble();
    static [[nodiscard]] Envy::string expand_column(column column, Envy::string macro, column_description desc);
    static [[nodiscard]] Envy::string color_str(color c);

    static [[nodiscard]] void build_column_color_cache();
    static [[nodiscard]] void determine_preamble_width();
    static [[nodiscard]] void build_header();

    static [[nodiscard]] Envy::string func_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string file_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string line_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string col_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string datetime_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string severity_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string severity_short_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string severity_color_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string logger_name_macro(Envy::string_view param);
    static [[nodiscard]] Envy::string clamp(Envy::string_view s, i32 width, alignment align, char fill);


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::logger ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    logger::logger(Envy::string name) noexcept :
        name {std::move(name)}
    { }


    //**********************************************************************
    logger::logger(Envy::string name, Envy::string log_file) noexcept :
        name     {std::move(name)},
        logfile  {std::move(log_file)}
    { }


    //**********************************************************************
    logger::logger(Envy::string name, Envy::string log_file, bool console) noexcept :
        name             { std::move(name) },
        logfile          { std::move(log_file) },
        console_logging  { console }
    { }


    //**********************************************************************
    message logger::make_message(severity sev, Envy::string_view fmt, std::source_location loc)
    {
        update_log_state(name, sev, loc);

        return message
        {
            *this,
            sev,
            std::move(loc),
            Envy::expand_macros(fmt, log_macros)
        };
    }


    //**********************************************************************
    void logger::assert(bool test, Envy::string_view msg, std::source_location loc)
    {
        if(!test)
        {
            update_log_state(name, severity::assert, loc);
            Envy::log::raw_log(logfile, console_logging, msg);
            throw Envy::assertion(msg, loc);
        }
    }


    //**********************************************************************
    void logger::debug_assert(bool test, Envy::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    {
        ENVY_DEBUG_CALL(assert(test,msg,loc));
    }


    //**********************************************************************
    message logger::error(Envy::string_view fmt, std::source_location loc)
    {
        return make_message(severity::error, fmt, loc);
    }


    //**********************************************************************
    message logger::warning(Envy::string_view fmt, std::source_location loc)
    {
        return make_message(severity::warning, fmt, loc);
    }


    //**********************************************************************
    message logger::note(Envy::string_view fmt, std::source_location loc)
    {
        return make_message(severity::note, fmt, loc);
    }


    //**********************************************************************
    message logger::info(Envy::string_view fmt, std::source_location loc)
    {
        return make_message(severity::info, fmt, loc);
    }


    //**********************************************************************
    void logger::print_header(Envy::string name)
    {
        if(console_logging)
        {
            std::scoped_lock l {console_mutex};

            // headers don't have a severity , Add Header color?
            if(desc.border_color == log::color::severity)
            {
                print(color_str(log::color::dark_gray));
            }
            else
            {
                print(color_str(desc.border_color));
            }

            printl(header_underline);
            printl(header + name);
            printl(header_underline);
        }

        if(!logfile.empty())
        {
            std::ofstream fs;
            fs.open(logfile, std::ios::app);

            fs << header_underline << '\n';
            fs << header << name <<'\n';
            fs << header_underline << '\n';

            fs.close();
        }
    }


    //**********************************************************************
    bool logger::logs_to_console() const noexcept
    { return console_logging; }


    //**********************************************************************
    Envy::string logger::get_file() const noexcept
    { return logfile; }


    //**********************************************************************
    void logger::enable_console_logging(bool b) noexcept
    { console_logging = b; }


    //**********************************************************************
    void logger::set_file(Envy::string file) noexcept
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
    Envy::string_view logger::get_name()
    { return name; }


    //**********************************************************************
    scope_logger::scope_logger(Envy::string msg, logger& l, std::source_location loc) :
        log {l}
    {
        update_log_state(log.get_name(), severity::scope, loc);
        raw_log(log.get_file(), log.logs_to_console(), expand_macros("{{ " + color_str(desc.border_color) + msg, log_macros));
        indent_log();
        t = std::chrono::high_resolution_clock::now();
    }


    //**********************************************************************
    scope_logger::~scope_logger()
    {
        std::chrono::duration<f64> delta { std::chrono::high_resolution_clock::now() - t };
        unindent_log();
        update_log_state(log.get_name(), severity::scope, {});
        raw_log(log.get_file(), log.logs_to_console(), expand_macros("} " + color_str(desc.border_color) + std::format("{}", delta), log_macros));
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Global log functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void init(const description& logdesc)
    {
        desc = logdesc;

        border_color = color_str(desc.border_color);
        message_color = color_str(desc.message_color);

        build_column_color_cache();
        determine_preamble_width();
        build_header();

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

        log_macros.add("MSG", [](Envy::string fmt){ return color_str(desc.message_color); });
        log_macros.add("BRD", [](Envy::string fmt){ return color_str(desc.border_color); });
        log_macros.add("SEV", [](Envy::string fmt){ return color_str(severity_colors[static_cast<u8>(msg_severity)]); });

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
    void update_log_state(Envy::string_view logger_name, severity sev, message_source loc)
    {
        msg_source = loc;
        msg_severity = sev;

        // TODO: Envy::string copy asign from std::strin and Envy::string_view
        msg_logger = Envy::string(logger_name);
    }


    //**********************************************************************
    Envy::string expand_log_macros(Envy::string_view str)
    {
        return Envy::expand_local_macros(str, log_macros);
    }


    //**********************************************************************
    i32 errors()
    { return error_count; }


    //**********************************************************************
    i32 warnings()
    { return warning_count; }


    //**********************************************************************
    void raw_log(Envy::string_view logger_file, bool log_to_console, Envy::string_view msg)
    {
        if(msg_severity == severity::error)
        { ++error_count; }
        else if(msg_severity == severity::warning)
        { ++warning_count; }

        // -- process message

        Envy::string preamble { build_preamble() };
        Envy::string logmsg { process_message(msg) };

        // -- log message

        if(log_to_console)
        {
            std::scoped_lock l {console_mutex};
            print(preamble);
            print(indent_string());
            print(logmsg);
        }

        if(!logger_file.empty())
        {
            std::ofstream fs;
            fs.open(logger_file, std::ios::app);

            // TODO: skip writing ansi escape sequences
            fs << preamble << indent_string() << logmsg;

            fs.close();
        }
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void build_column_color_cache()
    {
        column_colors.reserve(static_cast<u8>(column::COUNT));

        // The order of these is very important
        column_colors.push_back(color_str(desc.func_column_desc.color));
        column_colors.push_back(color_str(desc.file_column_desc.color));
        column_colors.push_back(color_str(desc.line_column_desc.color));
        column_colors.push_back(color_str(desc.col_column_desc.color));
        column_colors.push_back(color_str(desc.datetime_column_desc.color));
        column_colors.push_back(color_str(desc.logger_column_desc.color));
        column_colors.push_back(color_str(desc.severity_column_desc.color));
    }


    //**********************************************************************
    void determine_preamble_width()
    {
        preamble_width = 0;

        for(auto column : desc.preamble)
        {
            preamble_width += 2; // "| "

            switch(column)
            {
            case column::source_function:  preamble_width += desc.func_column_desc.width;     break;
            case column::source_file:      preamble_width += desc.file_column_desc.width;     break;
            case column::source_line:      preamble_width += desc.line_column_desc.width;     break;
            case column::source_column:    preamble_width += desc.col_column_desc.width;      break;
            case column::datetime:         preamble_width += desc.datetime_column_desc.width; break;
            case column::logger_name:      preamble_width += desc.logger_column_desc.width;   break;
            case column::severity:         preamble_width += desc.severity_column_desc.width; break;
            }

            preamble_width += 1; // " "
        }
        preamble_width += 4; // "| : "
    }


    //**********************************************************************
    void build_header()
    {
        header.clear();
        header_underline.clear();
        note_preamble.clear();

        header.reserve(preamble_width + 140);
        header_underline.reserve(preamble_width + 140);
        note_preamble.reserve(preamble_width);

        for(auto column : desc.preamble)
        {
            header += "| ";
            header_underline += "+-";
            note_preamble += "|";

            switch(column)
            {
            case column::source_function:
                header += clamp("Function", desc.func_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.func_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.func_column_desc.width+2u,'.');
                break;

            case column::source_file:
                header += clamp("File", desc.file_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.file_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.file_column_desc.width+2u,'.');
                break;

            case column::source_line:
                header += clamp("Line", desc.line_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.line_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.line_column_desc.width+2u,'.');
                break;

            case column::source_column:
                header += clamp("Col", desc.col_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.col_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.col_column_desc.width+2u,'.');
                break;

            case column::datetime:
                header += clamp("Datetime", desc.datetime_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.datetime_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.datetime_column_desc.width+2u,'.');
                break;

            case column::logger_name:
                header += clamp("Logger", desc.logger_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.logger_column_desc.width,'-');
                note_preamble += Envy::string ((std::size_t)desc.logger_column_desc.width+2u,'.');
                break;

            case column::severity:
                header += clamp("Severity", desc.severity_column_desc.width, alignment::left, ' ');
                header_underline += Envy::string ((std::size_t)desc.severity_column_desc.width,'-');

                msg_severity = log::severity::note;
                note_preamble += " " + clamp(log_macros.expand("severity",desc.severity_column_desc.fmt_spec), desc.severity_column_desc.width, desc.severity_column_desc.align, ' ') + " ";
                break;

            }

            header += " ";
            header_underline += "-";
        }

        header += "| ";
        header_underline += "+-" + Envy::string(130,'-');
        note_preamble += "| : ";
    }


    //**********************************************************************
    Envy::string build_preamble()
    {
        Envy::string preamble;

        for(auto column : desc.preamble)
        {
            preamble += color_str(desc.border_color) + "| ";

            switch(column)
            {
            case column::source_function:  preamble += expand_column(column::source_function, "func",     desc.func_column_desc);     break;
            case column::source_file:      preamble += expand_column(column::source_file,     "file",     desc.file_column_desc);     break;
            case column::source_line:      preamble += expand_column(column::source_line,     "line",     desc.line_column_desc);     break;
            case column::source_column:    preamble += expand_column(column::source_column,   "col",      desc.col_column_desc);      break;
            case column::datetime:         preamble += expand_column(column::datetime,        "datetime", desc.datetime_column_desc); break;
            case column::logger_name:      preamble += expand_column(column::logger_name,     "logger",   desc.logger_column_desc);   break;
            case column::severity:         preamble += expand_column(column::severity,        "severity", desc.severity_column_desc); break;
            }

            preamble += color_str(desc.border_color) + " ";
        }

        preamble += color_str(desc.border_color) + "| : " + message_color;

        return preamble;
    }


    //**********************************************************************
    Envy::string expand_column(column column, Envy::string macro, column_description desc)
    {
        auto expantion { log_macros.expand(macro, desc.fmt_spec) };

        // severity color changes per message, so we can't use the cached color string edit
        Envy::string color { /* (macro=="severity") ? */ color_str(desc.color) /* : column_colors[static_cast<u8>(column)] */ };

        return color + clamp(expantion, desc.width, desc.align, ' ');
    }


    //**********************************************************************
    Envy::string color_str(color c)
    {
        if(c == color::severity)
        {
            return color_str(severity_colors[static_cast<u8>(msg_severity)]);
        }
        else return "\x1b[" + Envy::to_string( static_cast<u8>(c) ) + "m";
    }


    //**********************************************************************
    Envy::string process_message(Envy::string_view msg)
    {
        Envy::string logmsg { Envy::string::reserve_tag , msg.size() * 2u };

        if(msg_severity == severity::note)
        {
            logmsg += color_str(desc.border_color);
        }
        else
        {
            logmsg += color_str(desc.message_color);
        }

        indent_log();

        // every newline is a note, so add the note preamble
        logmsg += Envy::replace(msg, "\n", "\n" + color_str(desc.border_color) + note_preamble + indent_string() + color_str(desc.border_color)) + "\x1b[0m\n";

        unindent_log();

        return logmsg;
    }


    //**********************************************************************
    Envy::string indent_string()
    {
        if(indent_count <= 0)
        { return ""; }

        Envy::string result;
        result.reserve(indent_count * desc.indent.size());

        result += color_str(desc.border_color);

        for(int i {}; i < indent_count; ++i)
        { result += desc.indent; }

        return result;
    }


    //**********************************************************************
    Envy::string func_macro(Envy::string_view param)
    {
        return std::format( build_fmt_str(param) , msg_source.func );
    }


    //**********************************************************************
    Envy::string file_macro(Envy::string_view param)
    {
       return std::format( build_fmt_str(param) , std::filesystem::path {msg_source.file} .filename().string() );
    }


    //**********************************************************************
    Envy::string line_macro(Envy::string_view param)
    {
        return std::format( build_fmt_str(param) , msg_source.line );
    }


    //**********************************************************************
    Envy::string col_macro(Envy::string_view param)
    {
        return std::format( build_fmt_str(param) , msg_source.col );
    }


    //**********************************************************************
    Envy::string datetime_macro(Envy::string_view param)
    {
        std::chrono::zoned_time time {std::chrono::current_zone(), std::chrono::system_clock::now()};

        return std::format( build_fmt_str(param) , time );
    }


    //**********************************************************************
    Envy::string severity_macro(Envy::string_view param)
    {
        constexpr const char* severities[]
        {"scope", "assert", "error", "warning", "note", "info"};

        return std::format( build_fmt_str(param) , severities[static_cast<i32>(msg_severity)] );
    }


    //**********************************************************************
    Envy::string severity_short_macro(Envy::string_view param)
    {
        constexpr const char* severities_short[]
        {"scp", "asr", "err", "wrn", "nte", "inf"};

        return std::format( build_fmt_str(param) , severities_short[static_cast<i32>(msg_severity)] );
    }


    //**********************************************************************
    Envy::string severity_color_macro(Envy::string_view param)
    {
        return color_str(severity_colors[static_cast<i32>(msg_severity)]);
    }


    //**********************************************************************
    Envy::string logger_name_macro(Envy::string_view param)
    {
        return std::format( build_fmt_str(param) , (std::string) msg_logger );
    }


    //**********************************************************************
    Envy::string clamp(Envy::string_view s, i32 width, alignment align, char fill)
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

            f64 a { static_cast<u8>(align) / 2.0 };
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

            if(align == alignment::right)
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

namespace Envy
{

    //**********************************************************************
    log::message error(Envy::string fmt, std::source_location loc)
    { return log::global.make_message(log::severity::error, std::move(fmt), loc); }


    //**********************************************************************
    log::message warning(Envy::string fmt, std::source_location loc)
    { return log::global.make_message(log::severity::warning, std::move(fmt), loc); }


    //**********************************************************************
    log::message note(Envy::string fmt, std::source_location loc)
    { return log::global.make_message(log::severity::note, std::move(fmt), loc); }


    //**********************************************************************
    log::message info(Envy::string fmt, std::source_location loc)
    { return log::global.make_message(log::severity::info, std::move(fmt), loc); }


    //**********************************************************************
    void assert(bool test, Envy::string_view msg, std::source_location loc)
    { log::global.assert(test,msg,loc); }


    //**********************************************************************
    void debug_assert(bool test, Envy::string_view msg, std::source_location loc) noexcept(!Envy::debug)
    { log::global.debug_assert(test,msg,loc); }

}