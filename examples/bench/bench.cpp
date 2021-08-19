#include <Envy/engine.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>
#include <Envy/window.hpp>
#include <Envy/event.hpp>
#include <Envy/vector.hpp>
#include <Envy/test.hpp>
#include <Envy/string.hpp>

#include <Windows.h>

#include <iostream>


int main(int argc, char** argv)
{
    Envy::init_logging();

    Envy::engine::description engdesc {};
    Envy::engine::run(engdesc, argc, argv);

    return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}