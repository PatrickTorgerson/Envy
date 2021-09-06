///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2021 Patrick Torgerson
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////

/********************************************************************************
 * \file engine.hpp
 * \brief Envy's entry point
 ********************************************************************************/

#pragma once

#include "common.hpp"
#include "log.hpp"
#include "window.hpp"
#include "graphics.hpp"

namespace Envy::engine
{
    /********************************************************************************
     * \brief Class containing information used to configure Envy
     *
     * To start the engine you must create one of these and pass it into
     * \ref Envy::engine::run().
     *
     * \see Envy::window::description
     * \see Envy::graphics::description
     ********************************************************************************/
    struct description
    {
        window::description window {};      ///< Used to configure the window
        graphics::description graphics {};  ///< Used to configure graphics options
        log::description log {};            ///< Used to configure logging options
        // cli
        // roots
        // rootstack
    };


    /********************************************************************************
     * \brief The entry point for Envy
     *
     * ```cpp
     * #include <Envy.hpp>
     *
     * int main()
     * {
     *      Envy::engine::description envydesc {};
     *
     *      envydesc.window.tile = L"My Envy App";
     *      envydesc.window.minimum_size = {500,500};
     *
     *      Envy::engine::run(envydesc);
     *
     *      return 0;
     * }
     * ```
     *
     * \param [in] engdesc A description object used to configure Envy
     * \param [in] argc Optional command line argument count, usually forwarded from main()
     * \param [in] argv Optional command line argument values, usually forwarded from main()
     *
     * \see Envy::engine::description
     * \see Envy::window::description
     * \see Envy::graphics::description
     ********************************************************************************/
    void run(const description& engdesc, int argc = 0, char** argv = nullptr);
}