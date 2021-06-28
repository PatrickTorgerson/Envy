#include <exception.hpp>

#include <format>

namespace Envy
{

    const std::string msgfmt {"[{1} LN{2}] : {0}"};

    exception::exception(std::string_view msg, std::source_location loc)
        : msg {std::format(msgfmt, msg, loc.file_name(), loc.line())}
    {}


    const char* exception::what() const
    { return msg.c_str(); }

}