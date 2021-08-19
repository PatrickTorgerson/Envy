#include <Envy/engine.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>
#include <Envy/window.hpp>
#include <Envy/event.hpp>
#include <Envy/vector.hpp>
#include <Envy/test.hpp>
#include <Envy/string.hpp>

#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <iostream>

#include "tests.hpp"


void run_tests();


int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);

    Envy::init_logging();

    run_tests();

    Envy::engine::description engdesc {};
    Envy::engine::run(engdesc, argc, argv);

    return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}


void run_tests()
{
    Envy::log.seperator();

    Envy::test_state tests;

    utf8_test(tests);
    string_test(tests);
    string_view_test(tests);
    vector_test(tests);
    unicode_test(tests);
    buffers_test(tests);
    macro_test(tests);

    tests.report();

    Envy::log.seperator();
}