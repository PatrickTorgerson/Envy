#include <Envy/application.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>
#include <Envy/window.hpp>

#include <Windows.h>


int main(int argc, char** argv)
{
    Envy::application app;

    app.run();

    return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}