#include <string.hpp>

#include <cstring>
#include <algorithm>

namespace Envy
{

    // string::string() :
    //     size     {0u},
    //     capacity {16u},
    //     bytes    {new u8[capacity]}
    // { std::fill(bytes.get(), bytes.get() + capacity, '\0'); }


    // string::string(const char* cstr) :
    //     size     {std::strlen(cstr)},
    //     capacity {size + 11u},
    //     bytes    {new u8[capacity]}
    // {
    //     strncpy_s( (char*) bytes.get(), capacity, cstr, capacity);
    // }


    // char* string::cstr() noexcept
    // { return (char*) bytes.get(); }


    // const char* string::cstr() const noexcept
    // { return (const char*) bytes.get(); }


    // u8* string::data() noexcept
    // { return bytes.get(); }


    // const u8* string::data() const noexcept
    // { return bytes.get(); }


    // usize string::data_size() const
    // { return size; }


    // std::ostream& operator << (std::ostream& os, const string& s)
    // {
    //     os << s.cstr();
    //     return os;
    // }

}