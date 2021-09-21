#include <Envy/engine.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>
#include <Envy/window.hpp>
#include <Envy/event.hpp>
#include <Envy/vector.hpp>
#include <Envy/test.hpp>
#include <Envy/string.hpp>

#include <iostream>

#include <Windows.h>

#include "tests.hpp"


void run_tests();


int main()
{
    SetConsoleOutputCP(CP_UTF8);


    Envy::engine::description engdesc {};

    engdesc.log.preamble =
    {
        Envy::log::column::datetime,
        Envy::log::column::source_file,
        Envy::log::column::source_line,
        Envy::log::column::logger_name
    };

    engdesc.log.logger_column_desc.color = Envy::log::color::severity;

    // pre-init logger for tests
    Envy::log::init(engdesc.log);

    run_tests();

    Envy::engine::run(engdesc);

    return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return main();
}


void run_tests()
{
    Envy::log::global.print_header(" Tests ");

    Envy::test_state tests {Envy::test_state::verbose};

    tests.get_logger().info("{BLK}████{BLU}████{GRN}████{CYN}████{RED}████{MAG}████{YEL}████{WHT}████");
    tests.get_logger().info("{DGRY}████{LBLU}████{LGRN}████{LCYN}████{LRED}████{LMAG}████{LYEL}████{LGRY}████");

    utf8_test(tests);
    string_test(tests);
    string_view_test(tests);
    vector_test(tests);
    unicode_test(tests);
    buffers_test(tests);
    macro_test(tests);

    tests.report();

    Envy::log::global.print_header();
}