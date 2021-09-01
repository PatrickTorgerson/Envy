#include <engine.hpp>

#include <graphics.hpp>
#include <event.hpp>

#include <thread>
#include <chrono>

namespace Envy::engine
{

    void run(const description& engdesc, int argc, char** argv)
    {
        std::chrono::high_resolution_clock::time_point start { std::chrono::high_resolution_clock::now() };

        // Launch message thread
        // responsible for creating the window, and handaling Windows Messages
        std::jthread msgthread {window::run, engdesc.window};

        // Pause execution until message thread has created the window
        // as graphics depends on window
        window::wait_for_creation();

        graphics::init(engdesc.graphics);

        // main loop
        while(window::is_open())
        {
            // update input state / dispach system events
            Envy::dispach_events();

            // update

            // render
            graphics::clear();
            graphics::present();
        }

        std::chrono::duration<f64> runtime { std::chrono::high_resolution_clock::now() - start };

        Envy::info("Application exited")
        .note("runtime: {}", runtime);

        Envy::log.seperator();
    }

}