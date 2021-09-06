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


/********************************************************************************
 * \file log.hpp
 * \brief Envy's logging library
 ********************************************************************************/

#pragma once

#include "common.hpp"
#include "string.hpp"

#include <format>
#include <source_location>
#include <filesystem>
#include <chrono>

namespace Envy::log
{

    /********************************************************************************
     * \brief Log severity levels
     ********************************************************************************/
    enum class severity : u8
    {
        scope,    //< Used by Envy to log scope specifiers
        assert,   //< Used by Envy to log assertion messages
        error,    //< Indicates a log message describes an error
        warning,  //< Indicates a log message describes a warning
        note,     //< Indicates a log message provides more information about a previouse log message
        info      //< Indicates a log message is providing information
    };


    /********************************************************************************
     * \brief Logger colors
     * \see Envy::log::description
     ********************************************************************************/
    enum class color : u8
    {
        black          =  30,
        red            =  31,
        green          =  32,
        yellow         =  33,
        blue           =  34,
        magenta        =  35,
        cyan           =  36,
        light_gray     =  37,
        dark_gray      =  90,
        light_red      =  91,
        light_green    =  92,
        light_yellow   =  93,
        light_blue     =  94,
        light_magenta  =  95,
        light_cyan     =  96,
        white          =  97,
        def            =  39,
        clear          =  0,
        severity       =  255
    };


    /********************************************************************************
     * \brief Logger alignment
     * \see Envy::log::description
     ********************************************************************************/
    enum class alignment : u8
    {
        left   = 0,
        center = 1,
        right  = 2
    };


    /********************************************************************************
     * \brief Logger column
     * \see Envy::log::description
     ********************************************************************************/
    enum class column : u8
    {
        source_function,
        source_file,
        source_line,
        source_column,
        datetime,
        logger_name,
        severity,
        COUNT
    };


    /********************************************************************************
     * \brief Configures how a log column should be formatted
     * \see Envy::log::description
     ********************************************************************************/
    struct column_description
    {
        i32 width;             ///< Fixed width of the column
        Envy::string fmt_spec;  ///< fmt specifier passed to std::format()
        alignment align;       ///< alignment of the text within the column
        color color;           ///< color of the text within the column
    };


    /********************************************************************************
     * \brief Allows you to configure logger output
     *
     * This is passed to Envy in a \ref Envy::engine::description object to
     * the \ref Envy::engine::run() function.
     *
     * \see Envy::engine::description
     * \see Envy::engine::run()
     ********************************************************************************/
    struct description
    {

        /********************************************************************************
         * \brief Specify what information you want to apear in the preamble
         *
         * | Type                               | Information                                      |
         * | ---------------------------------- | ------------------------------------------------ |
         * | Envy::log::column::source_function | Name of the function the message was logged from |
         * | Envy::log::column::source_file     | File the message was logged from                 |
         * | Envy::log::column::source_line     | Line the message was logged on                   |
         * | Envy::log::column::source_column   | Colume the message was logged on                 |
         * | Envy::log::column::datetime        | Date and Time the message was logged at          |
         * | Envy::log::column::logger_name     | Name of the logger that jogged the message       |
         * | Envy::log::column::severity        | Severity of the message                          |
         *
         * You can configure each column indevidually with their respective Envy::log::column_description.
         *
         * \see Envy::log::column_description
         * \see Envy::log::column
         ********************************************************************************/
        std::vector<column> preamble {column::datetime, column::source_file, column::source_line, column::severity};


        /********************************************************************************
         * \brief Configure how the function column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description func_column_desc {15, "", alignment::left,   color::dark_gray};


        /********************************************************************************
         * \brief Configure how the file column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description file_column_desc {15, "", alignment::right,  color::dark_gray};


        /********************************************************************************
         * \brief Configure how the line column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description line_column_desc {4, "04", alignment::right, color::dark_gray};


        /********************************************************************************
         * \brief Configure how the col column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description col_column_desc  {4, "04", alignment::right, color::dark_gray};


        /********************************************************************************
         * \brief Configure how the datetime column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description datetime_column_desc {16, "%T", alignment::left, color::dark_gray};


        /********************************************************************************
         * \brief Configure how the logger column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description logger_column_desc   {10, "", alignment::right, color::dark_gray};


        /********************************************************************************
         * \brief Configure how the severity column should be formatted
         * \see Envy::log::column_description
         ********************************************************************************/
        column_description severity_column_desc {8, "", alignment::right, color::severity};


        /********************************************************************************
         * \brief String to use per indent
         ********************************************************************************/
        Envy::string indent {"|   "};


        /********************************************************************************
         * \brief Color of border characters
         ********************************************************************************/
        color border_color {color::dark_gray};


        /********************************************************************************
         * \brief Color of message characters
         ********************************************************************************/
        color message_color {color::white};

        // std::vector<severity> severity_filter {severity::scope, severity::assert, severity::error, severity::warning, severity::note, severity::info};
    };


    class logger;
    class message;
    class message_source;


    /********************************************************************************
     * \brief Called by Envy to initialize the logging system
     *
     * Sets up log macros
     *
     * \see Envy::resolve()
     ********************************************************************************/
    void init(const description& logdesc);


    /********************************************************************************
     * \brief Increments log indent level
     *
     * Causes future messages logged to be printed with one more indent.
     * TODO: add logger specific indent level
     *
     * \see Envy::unindent_log()
     ********************************************************************************/
    void indent_log();


    /********************************************************************************
     * \brief Decrements log indent level
     *
     * Causes future messages logged to be printed with one less indent.
     * TODO: add logger specific indent level
     *
     * \see Envy::indent_log()
     ********************************************************************************/
    void unindent_log();


    /********************************************************************************
     * \brief Internaly used to log messages to the console and/or to a file
     *
     * Envy uses this internaly to do the logging, you likely will not need to use it
     * yourself. However if you wish to forward messages from third party libraries to
     * Envy's logging system this function can be used. You must remember to call
     * Envy::update_log_state() first though.
     *
     * \param [in] logger_file File to log to, leave empty to not log to a file
     * \param [in] log_to_console Whether this message should be logged to the console
     * \param [in] msg The message to log
     *
     * \see Envy::logger
     * \see Envy::error()
     * \see Envy::warning()
     * \see Envy::note()
     * \see Envy::info()
     ********************************************************************************/
    void raw_log(Envy::string_view logger_file, bool log_to_console, Envy::string_view msg);


    /********************************************************************************
     * \brief Updates the log message state
     *
     * \param [in] logger_name Name of the logger logging the next message
     * \param [in] sev Severity of the next message
     * \param [in] loc Source location of the next message
     ********************************************************************************/
    void update_log_state(Envy::string_view logger_name, severity sev, message_source loc);


    /********************************************************************************
     * \brief Expand log macros found in string
     *
     * \param [in] str String to expand
     * \return Envy::string
     ********************************************************************************/
    Envy::string expand_log_macros(Envy::string_view str);


    /********************************************************************************
     * \brief Return current number of error messages
     ********************************************************************************/
    i32 errors();


    /********************************************************************************
     * \brief Return current number of warning messages
     ********************************************************************************/
    i32 warnings();


    /********************************************************************************
     * \brief Contains information about the source location of a log_message
     ********************************************************************************/
    class message_source
    {
    public:

        const char* file {""};  ///< The file the message was logged from
        const char* func {""};  ///< The function the message was logged in
        u32 line {0u};          ///< The line the message was logged on
        u32 col  {0u};          ///< The column the message was logged on


        /********************************************************************************
         * \brief Constructs a log_message_source from a std::source_location
         ********************************************************************************/
        message_source(std::source_location loc = std::source_location::current()) :
            file { loc.file_name() },
            func { loc.function_name() },
            line { loc.line() },
            col  { loc.column() }
        { }


        /********************************************************************************
         * \brief Constructs a log_message_source
         ********************************************************************************/
        message_source(const char* file, const char* func, u32 line, u32 col) :
            file { file },
            func { func },
            line { line },
            col  { col }
        { }
    };


    /********************************************************************************
     * \brief Used to log to the console and/or a file
     ********************************************************************************/
    class logger
    {

        Envy::string name {};          ///< Name of the logger, can be displayed in a log preamble
        Envy::string logfile {};       ///< File to log to, empty for no file
        bool console_logging {true};  ///< Whether messages should be logged to the console

    public:

        /********************************************************************************
         * \brief Constructs logger that only logs to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         ********************************************************************************/
        explicit logger(Envy::string name) noexcept;


        /********************************************************************************
         * \brief Constructs a logger that logs to a file and to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         * \param [in] log_file File to log to
         ********************************************************************************/
        logger(Envy::string name, Envy::string log_file) noexcept;


        /********************************************************************************
         * \brief Constructs a logger that logs to a file and optionally to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         * \param [in] log_file File to log to
         * \param [in] console hether messages should be logged to the console
         ********************************************************************************/
        logger(Envy::string name, Envy::string log_file, bool console) noexcept;


        /********************************************************************************
         * \brief Creates a log message
         *
         * Envy uses this internally to create log messages that can be formatted with
         * \ref Envy::log_message::operator()(), noted with \ref Envy::log_message::note(),
         * and will be logged on destruction. You should not have to use this function directly.
         * It is public so \ref Envy::error(), \ref Envy::warning(), \ref Envy::note(), and
         * \ref Envy::info() can use it.
         *
         * \param [in] severity The log severity of the log message
         * \param [in] fmt A format string, can be formatted with \ref Envy::log_message::operator()()
         * \param [in] loc \ref std::source_location describing where the message was created
         *
         * \return \ref Envy::log_message
         ********************************************************************************/
        message make_message(severity severity, Envy::string_view fmt, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Logs an error message
         *
         * Returns a helper type to allow you to format the message using \ref std::format()
         *
         * `logger.error("File '{}' not found!")(filename);`
         *
         * You can also add note with .note()
         *
         * `logger.error().note()`
         *
         * \param [in] fmt The format string, pass argument in parens after function call
         * \param [in] loc \ref std::source_location describing where the message was logged from
         *
         * \return Envy::log_message Format the message with function call operator
         ********************************************************************************/
        message error(Envy::string_view fmt, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Logs a warning message
         *
         * Returns a helper type to allow you to format the message using \ref std::format()
         *
         * `logger.warning("Could not find config file '{}', loading default config")(configfile);`
         *
         * You can also add note with .note()
         *
         * `logger.warning().note()`
         *
         * \param [in] fmt The format string, pass argument in parens after function call
         * \param [in] loc \ref std::source_location describing where the message was logged from
         *
         * \return Envy::log_message Format the message with function call operator
         ********************************************************************************/
        message warning(Envy::string_view fmt, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Logs a note message
         *
         * Returns a helper type to allow you to format the message using \ref std::format()
         *
         * `logger.info("Window resized")`
         *
         * `logger.note("New size {}")(winresize.size);`
         *
         * You can also add note with .note()
         *
         * `logger.note().note()`
         *
         * \param [in] fmt The format string, pass argument in parens after function call
         * \param [in] loc \ref std::source_location describing where the message was logged from
         *
         * \return Envy::log_message Format the message with function call operator
         ********************************************************************************/
        message note(Envy::string_view fmt, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Logs an information message
         *
         * Returns a helper type to allow you to format the message using \ref std::format()
         *
         * `logger.info("Envy version: {}.{}.{}")(Envy::version_major, Envy::version_minor, Envy::version_patch);`
         *
         * You can also add note with .note()
         *
         * `logger.info().note()`
         *
         * \param [in] fmt The format string, pass argument in parens after function call
         * \param [in] loc \ref std::source_location describing where the message was logged from
         *
         * \return Envy::log_message Format the message with function call operator
         ********************************************************************************/
        message info(Envy::string_view fmt, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Asserts that a condition is true
         *
         * If the assertion fails the given message will be logged and an \ref Envy::assertion
         * will be thrown. It is expected that this assertion exception will be unhandled and
         * cause the program to crash.
         *
         * \param [in] test Condition to test
         * \param [in] msg Message to log if the test fails
         * \param [in] loc \ref std::source_location describing where the assertion occured
         ********************************************************************************/
        void assert(bool test, Envy::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief Asserts that a condition is true on dubug builds only
         *
         * If the assertion fails the given message will be logged and an \ref Envy::assertion
         * will be thrown. It is expected that this assertion exception will be unhandled and
         * cause the program to crash.
         *
         * \param [in] test Condition to test
         * \param [in] msg Message to log if the test fails
         * \param [in] loc \ref std::source_location describing where the assertion occured
         ********************************************************************************/
        void debug_assert(bool test, Envy::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);


        /********************************************************************************
         * \brief Whether the logger is logging to the console
         *
         * \return true if this logger is logging to the console, false otherwise
         *
         * \see Envy::logger::enable_console_logging()
         ********************************************************************************/
        bool logs_to_console() const noexcept;


        /********************************************************************************
         * \brief Turns console logging on or off
         *
         * \param [in] b true if the logger should log to the console, false otherwise
         *
         * \see Envy::logger::logs_to_console()
         ********************************************************************************/
        void enable_console_logging(bool b) noexcept;


        /********************************************************************************
         * \brief Logs a seperator on it's own line
         *
         * TODO: Envy::set_log_seperator()
         ********************************************************************************/
        void print_header(Envy::string name = "Message");


        /********************************************************************************
         * \brief Returns the file path this logger is logging to
         *
         * \return Envy::string the file path
         *
         * \see Envy::logger::set_file()
         * \see Envy::logger::clear_file()
         ********************************************************************************/
        Envy::string get_file() const noexcept;


        /********************************************************************************
         * \brief Sets the file the logger should log to
         *
         * \param [in] file The file the logger should log to
         *
         * \see Envy::logger::get_file()
         * \see Envy::logger::clear_file()
         ********************************************************************************/
        void set_file(Envy::string file) noexcept;


        /********************************************************************************
         * \brief Deletes the content of the logger's log file
         *
         * \see Envy::logger::set_file()
         * \see Envy::logger::get_file()
         ********************************************************************************/
        void clear_file();


        /********************************************************************************
         * \brief Returns the name of the logger
         *
         * TODO: return Envy::string??
         *
         * \return Envy::string_view The name of the logger
         ********************************************************************************/
        Envy::string_view get_name();
    };


    /********************************************************************************
     * \brief The global logger
     *
     * It is recomended that yopu use the global logging function to log with this logger.
     * You can however log to it directly. The following lines are effectivly equivilent.
     *
     * `Envy::log.info("Hello there. {}")("General Kenobi");`
     *
     * `Envy::info("Hello there. {}")("General Kenobi");`
     *
     * \see Envy::info()
     * \see Envy::note()
     * \see Envy::warning()
     * \see Envy::error()
     * \see Envy::logger
     ********************************************************************************/
    inline logger global {"Envy"};


    /********************************************************************************
     * \brief Helper class for formatting log messages and adding notes
     * \see Envy::logger
     * \see Envy::error()
     * \see Envy::warning()
     * \see Envy::note()
     * \see Envy::info()
     ********************************************************************************/
    class message
    {

        logger& log;               //< The logger the message will be logged with
        severity sev;              //< The log severity of the message
        std::source_location loc;  //< Location where the message was logged
        Envy::string fmt;           //< The format string, can be formatted with the function call operator

    public:

        /********************************************************************************
         * \brief Constructs a log_message
         *
         * Typicaly you will use one of \ref Envy::logger 's log functions wich
         * will return a \ref Envy::log_message. You shouldn't need to construct one yourself
         *
         * \param [in] log The logger the message will be logged with
         * \param [in] severity The log severity of the message
         * \param [in] loc Location where the message was logged
         * \param [in] fmt The format string, can be formatted with the function call operator
         *
         * \see Envy::logger
         * \see Envy::error()
         * \see Envy::warning()
         * \see Envy::note()
         * \see Envy::info()
         ********************************************************************************/
        message(logger& log, severity sev, std::source_location loc, Envy::string fmt) noexcept :
            log {log},
            sev {sev},
            loc {std::move(loc)},
            fmt {std::move(fmt)}
        { }


        /********************************************************************************
         * \brief Destructor, this is where the message actually gets logged
         ********************************************************************************/
        ~message()
        {
            // TODO: test on release build
            raw_log(log.get_file(), log.logs_to_console(), fmt);
        }


        message(const message&) = delete;
        message(message&&) = delete;
        message& operator=(const message&) = delete;
        message& operator=(message&&) = delete;


        /********************************************************************************
         * \brief Format the message's format string
         *
         * Forwards args to \ref std::format(), formatting member string *fmt*, wich
         * will be logged on destruction of the \ref Envy::log_message.
         * Also returns a reference to this for calling \ref Envy::log_message::note()
         *
         * \tparam Ts Argument pack, arguments must be convertable to string as define by \ref Envy::convertable_to_string
         * \param [in] args Arguments to be forwarded to \ref std::format() with member *fmt*
         * \return log_message& Refference to this log_message
         ********************************************************************************/
        template <convertable_to_string ... Ts>
        message& operator()(Ts&& ... args)
        {
            fmt = std::format( fmt, std::forward<Ts>(args)... );
            return *this;
        }


        /********************************************************************************
         * \brief Logs a note with this message
         *
         * Add a note for the current log message, will be logged on a new line with an indent.
         *
         * `Envy::error("File not found '{}'")(file).note("Current working directory '{}'", cwd)`
         *
         * \tparam Ts Argument pack, arguments must be convertable to string as define by \ref Envy::convertable_to_string
         * \param [in] fmtstr Format string of the notes message
         * \param [in] args Arguments to be forwarded to \ref std::format() with *fmtstr*
         * \return log_message& Refference to this log_message, to chain .note() calls
         ********************************************************************************/
        template <convertable_to_string ... Ts>
        message& note(Envy::string_view fmtstr, Ts&& ... args)
        {
            // logging system interprets new-lines as a new note
            fmt += "\n" + std::format(expand_log_macros(fmtstr), std::forward<Ts>(args)...);
            return *this;
        }

    };


    /********************************************************************************
     * \brief Logs, times, and indents the enclosing scope.
     *
     * Indents all log messages logged during it's life-time.
     * On destruction, logs the time in seconds it was alive.
     *
     * \see Envy::logger
     * \see Envy::info()
     * \see Envy::note()
     * \see Envy::warning()
     * \see Envy::error()
     ********************************************************************************/
    class scope_logger
    {
        logger& log; ///< Logger to use to log open and close scope messages
        std::chrono::high_resolution_clock::time_point t; ///< The time point the scope_logger was constructed
    public:


        /********************************************************************************
         * \brief Constructs a scope_logger
         *
         * All messages logged after construction and before destruction will be indented.
         * The destructor will unindent and log the time in seconds it took for the scope to execute
         *
         * \param [in] msg Message to log with the open scope message
         * \param [in] l Logger to use to log open and close scope messages
         * \param [in] loc Location the scope_logger was created
         ********************************************************************************/
        explicit scope_logger(Envy::string msg, logger& l = Envy::log::global, std::source_location loc = std::source_location::current());


        /********************************************************************************
         * \brief scope_logger's destructor
         *
         * Ends logging of the scope, logs time since construction in seconds.
         ********************************************************************************/
        ~scope_logger();

    };

}

namespace Envy
{
    using logger = Envy::log::logger;
    using scope_logger = Envy::log::scope_logger;


    /********************************************************************************
     * \brief Logs an error message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log::message Format the message with function call operator
     *
     * \see Envy::logger::error()
     ********************************************************************************/
    log::message error(Envy::string fmt, std::source_location loc = std::source_location::current());


    /********************************************************************************
     * \brief Logs a warning message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log::message Format the message with function call operator
     *
     * \see Envy::logger::warning()
     ********************************************************************************/
    log::message warning(Envy::string fmt, std::source_location loc = std::source_location::current());


    /********************************************************************************
     * \brief Logs a note message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log::message Format the message with function call operator
     *
     * \see Envy::logger::note()
     * \see Envy::log::message::note()
     ********************************************************************************/
    log::message note(Envy::string fmt, std::source_location loc = std::source_location::current());


    /********************************************************************************
     * \brief Logs an info message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log::message Format the message with function call operator
     *
     * \see Envy::logger::info()
     ********************************************************************************/
    log::message info(Envy::string fmt, std::source_location loc = std::source_location::current());


    /********************************************************************************
     * \brief Asserts that a condition is true, logs with global logger
     *
     * If the assertion fails the given message will be logged and an \ref Envy::assertion
     * will be thrown. It is expected that this assertion exception will be unhandled and
     * cause the program to crash.
     *
     * \param [in] test Condition to test
     * \param [in] msg Message to log if the test fails
     * \param [in] loc \ref std::source_location describing where the assertion occured
     *
     * \see Envy::logger::assert()
     ********************************************************************************/
    void assert(bool test, Envy::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());


    /********************************************************************************
     * \brief Asserts that a condition is true in debug builds only, logs with global logger
     *
     * If the assertion fails the given message will be logged and an \ref Envy::assertion
     * will be thrown. It is expected that this assertion exception will be unhandled and
     * cause the program to crash.
     *
     * \param [in] test Condition to test
     * \param [in] msg Message to log if the test fails
     * \param [in] loc \ref std::source_location describing where the assertion occured
     *
     * \see Envy::logger::debug_assert()
     ********************************************************************************/
    void debug_assert(bool test, Envy::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);


    /********************************************************************************
     * \brief Prints a value to the console without a newline
     *
     * Generally you will not use this function. Envy uses it internally to print
     * log messages
     *
     * \tparam T The type, must satisfy the concept \ref Envy::convertable_to_string
     * \param [in] v The value to be printed to the console
     *
     * \see Envy::printl()
     * \see Envy::convertable_to_string
     ********************************************************************************/
    template <convertable_to_string T>
    void print(T&& v)
    {
        if constexpr (std::same_as<T,bool>)
        { std::cout << std::boolalpha << v; }

        // else if constexpr (stream_insertable<T>)
        // { std::cout << std::forward<T>(v); }

        else
        { std::cout << Envy::to_string(std::forward<T>(v)); }
    }


    /********************************************************************************
     * \brief Prints a value to the console with a newline
     *
     * Generally you will not use this function. Envy uses it internally to print
     * log messages
     *
     * \tparam T The type, must satisfy the concept \ref Envy::convertable_to_string
     * \param [in] v The value to be printed to the console
     *
     * \see Envy::print()
     * \see Envy::convertable_to_string
     ********************************************************************************/
    template <convertable_to_string T>
    void printl(T&& v)
    { print(std::forward<T>(v)); print('\n'); }
}