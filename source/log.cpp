#include <log.hpp>

#include <string>
#include <format>
#include <iostream>
#include <exception>

#include <string_util.hpp>

namespace Envy
{
    namespace
    {
        std::string log_preamble {};
    }


    std::string resolve_preamble(std::source_location loc)
    {
        if(log_preamble.empty()) return log_preamble;

        std::string resolved {log_preamble};

        // replace_inplace(resolved, "{file}", "{0}");
        // replace_inplace(resolved, "{line}", "{1}");
        // replace_inplace(resolved, "{col}" , "{2}");
        // replace_inplace(resolved, "{func}", "{3}");

        // resolved = std::format(resolved, loc.file_name(), loc.line(), loc.column(), loc.function_name());

        return resolved;
    }


    void set_log_preamble(std::string preamble) noexcept
    {
        log_preamble = std::move(preamble);
    }


    void out(std::string_view msg, std::source_location loc)
    {
        std::cout << "OUT > " << resolve_preamble(loc) << msg << '\n';
    }


    void err(std::string_view msg, std::source_location loc)
    {
        std::cout << "ERR ! " << resolve_preamble(loc) << msg << '\n';
    }


    void wrn(std::string_view msg, std::source_location loc)
    {
        std::cout << "WRN ! " << resolve_preamble(loc) << msg << '\n';
    }

}