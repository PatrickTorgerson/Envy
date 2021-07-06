#include <Envy/application.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>

i64 factorial(i64 i)
{
    if(i == 1) return 1;
    else return i * factorial(i-1);
}

int main(int argc, char** argv)
{
    Envy::application app;

    return 0;
}