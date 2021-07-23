#include <application.hpp>

#include <log.hpp>

#include "window.hpp"

namespace Envy
{

    application::application()
    {
        Envy::init_logging();

        msg_thread = std::jthread(run_window);
    }


     void application::run()
     {
        while(window_is_open())
        {
            // do the thing
        }
     }

}