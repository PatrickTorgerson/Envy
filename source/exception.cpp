#include <exception.hpp>

#include <format>

namespace Envy
{

    const std::string msgfmt {"[{} LN{}] {}: {}"};

    exception::exception(std::string_view msg, std::source_location loc)
        : msg { std::format(msgfmt, loc.file_name(), loc.line(), "Exception", msg) }
    {}


    const char* exception::what() const
    { return msg.c_str(); }


    assertion::assertion(std::string_view msg, std::source_location loc)
        : msg { std::format(msgfmt, loc.file_name(), loc.line(), "Assertion", msg) }
    {}


    const char* assertion::what() const
    { return msg.c_str(); }

}