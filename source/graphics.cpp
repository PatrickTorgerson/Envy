#include <graphics.hpp>

#include <window.hpp>
#include <event.hpp>

#include <algorithm>

namespace Envy::graphics
{

    namespace
    {
    }


    void resize(const window::resized& resize);


    void init(const description& gfxdesc)
    {
        Envy::info("Graphics successfully initialized!")();

        // event callbacks

        Envy::register_callback<window::resized>(resize);
    }


    void clear()
    {
    }


    void present()
    {
    }


    void resize(const window::resized& resize)
    {
    }

}