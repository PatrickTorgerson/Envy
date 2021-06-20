#include <Envy/unicode.hpp>
#include <Envy/string.hpp>

#include <Windows.h>

#include <concepts>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

#include <Envy/buffers.hpp>


template <typename T>
concept numeric =
    std::floating_point<T> || std::integral<T>;


template <typename T>
concept streamy = requires (T t) { std::cout << t; };


std::string to_string(streamy auto v)
{
    using type = decltype(v);

    if constexpr (std::same_as<type,bool>)
    { return v ? "true" : "false"; }
    else if constexpr (std::same_as<type,char>)
    { return std::string(&v,1); }
    // TODO: unicode characters
    else if constexpr (numeric<type>)
    { return std::to_string(v); }
    else
    {
        // not ideal ...
        std::stringstream ss;
        ss << v;
        return std::move(ss).str();
    }
}


template <typename T>
concept stringy = requires (T t) { to_string(t); };


template <stringy T>
void print(T v)
{
    if constexpr (std::same_as<T,bool>)
    { std::cout << std::boolalpha << v; }
    else if constexpr (streamy<T>)
    { std::cout << v; }
    else
    { std::cout << to_string(v); }
}


void printl(stringy auto v)
{ print(v); print('\n'); }


class cosa { public: int data {}; std::string to_string() { return ::to_string(data); } };


int main(int argc, char** argv)
{
    SetConsoleOutputCP(65001);

    printl(5);
    printl(5.1);
    printl(true);
    printl("beans");

    cosa c;
    c.data = 12;

    print(c);

    return 0;
}