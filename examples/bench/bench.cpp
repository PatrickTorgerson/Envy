#include <Envy/log.hpp>

#include <Windows.h>

#include <chrono>
#include <format>

std::string date_macro()
{
    std::chrono::zoned_time t { std::chrono::current_zone() , std::chrono::system_clock::now() };
    return std::format("{:%m-%d-%Y %T}",t);
}

int main(int argc, char** argv)
{
    SetConsoleOutputCP(65001);

    Envy::macro("date_time", date_macro);

    Envy::printl( Envy::resolve(" {date_time} ") );

    return 0;
}