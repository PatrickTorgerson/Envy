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

// TODO: noexcept logging

namespace Envy
{

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

        else if constexpr (stream_insertable<T>)
        { std::cout << std::forward<T>(v); }

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

    /********************************************************************************
     * \brief Log severity levels
     ********************************************************************************/
    enum class log_severity : u8
    {
        scope,    //< Used by Envy to log scope specifiers
        assert,   //< Used by Envy to log assertion messages
        error,    //< Indicates a log message describes an error
        warning,  //< Indicates a log message describes a warning
        note,     //< Indicates a log message provides more information about a previouse log message
        info      //< Indicates a log message is providing information
    };

    /********************************************************************************
     * \brief Internaly used to log messages to the console and/or to a file
     *
     * Envy uses this internaly to do the logging, you likely will not need to use it
     * yourself. However if you wish to forward messages from third party libraries to
     * Envy's logging system this function can be used.
     *
     * \param [in] logger_name Name of the logger used to log this message
     * \param [in] logger_file File to log to, leave empty to not log to a file
     * \param [in] log_to_console Whether this message should be logged to the console
     * \param [in] severity The log severity of the log message
     * \param [in] msg The message to log
     * \param [in] file The file the message was logged from
     * \param [in] line The line the message was logged on
     * \param [in] col The column the message was logged on
     * \param [in] func The function the message was logged in
     *
     * \see Envy::logger
     * \see Envy::error()
     * \see Envy::warning()
     * \see Envy::note()
     * \see Envy::info()
     ********************************************************************************/
    void raw_log(std::string_view logger_name, std::string_view logger_file, bool log_to_console, log_severity severity, std::string_view msg, const char* file, u32 line, u32 col, const char* func);


    class log_message;

    /********************************************************************************
     * \brief Used to log to the console and/or a file
     ********************************************************************************/
    class logger
    {

        std::string name {};          ///< Name of the logger, can be displayed in a log preamble
        std::string logfile {};       ///< File to log to, empty for no file
        bool console_logging {true};  ///< Whether messages should be logged to the console

    public:

        /********************************************************************************
         * \brief Constructs logger that only logs to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         ********************************************************************************/
        explicit logger(std::string name) noexcept;

        /********************************************************************************
         * \brief Constructs a logger that logs to a file and to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         * \param [in] log_file File to log to
         ********************************************************************************/
        logger(std::string name, std::string log_file) noexcept;

        /********************************************************************************
         * \brief Constructs a logger that logs to a file and optionally to the console
         *
         * \param [in] name Name of the logger, can be displayed in a log preamble
         * \param [in] log_file File to log to
         * \param [in] console hether messages should be logged to the console
         ********************************************************************************/
        logger(std::string name, std::string log_file, bool console) noexcept;

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
        log_message message(log_severity severity, std::string_view fmt, std::source_location loc = std::source_location::current());

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
        log_message error(std::string_view fmt, std::source_location loc = std::source_location::current());

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
        log_message warning(std::string_view fmt, std::source_location loc = std::source_location::current());

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
        log_message note(std::string_view fmt, std::source_location loc = std::source_location::current());

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
        log_message info(std::string_view fmt, std::source_location loc = std::source_location::current());


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
        void assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());

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
        void debug_assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);

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
        void seperator();

        /********************************************************************************
         * \brief Returns the file path this logger is logging to
         *
         * \return std::string the file path
         *
         * \see Envy::logger::set_file()
         * \see Envy::logger::clear_file()
         ********************************************************************************/
        std::string get_file() const noexcept;

        /********************************************************************************
         * \brief Sets the file the logger should log to
         *
         * \param [in] file The file the logger should log to
         *
         * \see Envy::logger::get_file()
         * \see Envy::logger::clear_file()
         ********************************************************************************/
        void set_file(std::string file) noexcept;

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
         * TODO: return std::string??
         *
         * \return std::string_view The name of the logger
         ********************************************************************************/
        std::string_view get_name();
    };

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
     * \brief Helper class for formatting log messages and adding notes
     * \see Envy::logger
     * \see Envy::error()
     * \see Envy::warning()
     * \see Envy::note()
     * \see Envy::info()
     ********************************************************************************/
    class log_message
    {

        logger& log;               //< The logger the message will be logged with
        log_severity severity;     //< The log severity of the message
        std::source_location loc;  //< Location where the message was logged
        std::string fmt;           //< The format string, can be formatted with the function call operator

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
        log_message(logger& log, log_severity severity, std::source_location loc, std::string fmt) noexcept :
            log {log},
            severity {severity},
            loc {std::move(loc)},
            fmt {std::move(fmt)}
        { }

        /********************************************************************************
         * \brief Destructor, this is where the message actually gets logged
         ********************************************************************************/
        ~log_message()
        {
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), severity, fmt, loc.file_name(), loc.line(), loc.column(), loc.function_name());
        }

        log_message(const log_message&) = delete;
        log_message(log_message&&) = delete;
        log_message& operator=(const log_message&) = delete;
        log_message& operator=(log_message&&) = delete;

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
        log_message& operator()(Ts&& ... args)
        {
            fmt = std::format( fmt, std::forward<Ts>(args)... );
            return *this;
            //raw_log(log.get_name(), log.get_file(), log.logs_to_console(), severity, std::format( fmt, std::forward<Ts>(args)... ), loc.file_name(), loc.line(), loc.column(), loc.function_name());
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
        log_message& note(const std::string_view& fmtstr, Ts&& ... args)
        {
            // logging system interprets new-lines as a new note
            fmt += "\n" + std::format(fmtstr, std::forward<Ts>(args)...);
            return *this;
        }
    };

    /********************************************************************************
     * \brief The global logger
     *
     * It is recomended that yopu use the global logging function to log with this logger.
     * You can however log to it directly. The following lines are effectivly equivilent.
     *
     * `Envy::log.info("Hello there. {}")("General Kenobi");`
     * `Envy::info("Hello there. {}")("General Kenobi");`
     *
     * \see Envy::info()
     * \see Envy::note()
     * \see Envy::warning()
     * \see Envy::error()
     * \see Envy::logger
     ********************************************************************************/
    inline logger log {"Envy"};

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
        explicit scope_logger(std::string msg, logger& l = Envy::log, std::source_location loc = std::source_location::current()) :
            log {l}
        {
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), log_severity::scope, "{ {BBLK}" + msg, loc.file_name(), loc.line(), loc.column(), loc.function_name());
            indent_log();
            t = std::chrono::high_resolution_clock::now();
        }

        /********************************************************************************
         * \brief scope_logger's destructor
         *
         * Ends logging of the scope, logs time since construction in seconds.
         ********************************************************************************/
        ~scope_logger()
        {
            std::chrono::duration<f64> delta { std::chrono::high_resolution_clock::now() - t };
            unindent_log();
            raw_log(log.get_name(), log.get_file(), log.logs_to_console(), log_severity::scope, "} " + std::format("{{BBLK}}{}{{BWHT}}", delta), "", 0, 0, "");
        }
    };

    /********************************************************************************
     * \brief Called by Envy to initialize the logging system
     *
     * Sets up color macros
     *
     * \see Envy::resolve()
     ********************************************************************************/
    void init_logging();

    /********************************************************************************
     * \brief Logs an error message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log_message Format the message with function call operator
     *
     * \see Envy::logger::error()
     ********************************************************************************/
    log_message error(std::string fmt, std::source_location loc = std::source_location::current());

    /********************************************************************************
     * \brief Logs a warning message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log_message Format the message with function call operator
     *
     * \see Envy::logger::warning()
     ********************************************************************************/
    log_message warning(std::string fmt, std::source_location loc = std::source_location::current());

    /********************************************************************************
     * \brief Logs a note message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log_message Format the message with function call operator
     *
     * \see Envy::logger::note()
     * \see Envy::log_message::note()
     ********************************************************************************/
    log_message note(std::string fmt, std::source_location loc = std::source_location::current());

    /********************************************************************************
     * \brief Logs an info message with the global logger
     *
     * \param [in] fmt The format string, pass argument in parens after function call
     * \param [in] loc \ref std::source_location describing where the message was logged from
     * \return Envy::log_message Format the message with function call operator
     *
     * \see Envy::logger::info()
     ********************************************************************************/
    log_message info(std::string fmt, std::source_location loc = std::source_location::current());

    /********************************************************************************
     * \brief Sets the preamble pattern
     *
     * This function allows you to change what information is printed before every
     * log message. The preamble is a string that can contain; regular text, which will
     * be logged as-is; zero or more preamble macros, which will be expanded with
     * message-specific information; and zero or more color macros, which will change the color
     * of text printed to the console.
     *
     * Here are the available preamble macros:
     *
     *  - **{file}** : The file the message was logged from
     *  - **{line}** : Line number the message logged from
     *  - **{col}** : Column number the message logged from
     *  - **{func}** : Function name that the message was logged in
     *  - **{severity_short}** : Abreiviated log severity name (scp, ast, err, wrn, nte, inf)
     *  - **{severity}** : Log severity name (scope, assert, error, warning, note, info)
     *  - **{severity_color}** : Color macro for the log severity
     *  - **{datetime}** : Date and time the message was logged
     *  - **{name}** : Name of the logger used to log the message
     *
     * Here are the available color macros:
     *
     *  - **{RED}** : Red
     *  - **{GRN}** : Green
     *  - **{BLU}** : Blue
     *  - **{YEL}** : Yellow
     *  - **{CYN}** : Cyan
     *  - **{MAG}** : Magenta
     *  - **{WHT}** : White
     *  - **{BLK}** : Black
     *  - **{BRED}** : Bright Red
     *  - **{BGRN}** : Bright Green
     *  - **{BBLU}** : Bright Blue
     *  - **{BYEL}** : Bright Yellow
     *  - **{BCYN}** : Bright Cyan
     *  - **{BMAG}** : Bright Magenta
     *  - **{BWHT}** : Bright White
     *  - **{BBLK}** : Bright Black (Gray)
     *  - **{CLR}** : Default console color
     *
     * Actual color may vary depending on your terminal settings
     *
     * Macros are expanded using \ref std::format() and as such can include a [format specifier](https://en.cppreference.com/w/cpp/utility/format/formatter#Standard_format_specification)
     * here is an example of a preamble pattern:
     *
     * `"{BBLK}[{datetime}] {file: >20} LN{line:0>4} | {name: >10} {severity_color}{severity: >7}{BBLK} : {BWHT}"`
     *
     * which would resolve to something like:
     *
     * `"[08-23-2021 21:13:17.3582186]         "window.cpp" LN0074 |       Envy   info : "`
     *
     * \param [in] pattern A string containing option preamble macros
     ********************************************************************************/
    void set_preamble_pattern(std::string_view pattern);

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
    void assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current());

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
    void debug_assert(bool test, std::string_view msg = "Assertion failed", std::source_location loc = std::source_location::current()) noexcept(!Envy::debug);

}