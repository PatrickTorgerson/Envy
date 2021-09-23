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
 * \file window.hpp
 * \brief Utilities for Window creation and configuration
 ********************************************************************************/

#pragma once

#include "common.hpp"

#include "log.hpp"
#include "vector.hpp"
#include "event.hpp"

#include "win32.hpp"

#include <thread>

namespace Envy::window
{

    /********************************************************************************
     * \brief Class containing information used to configure the Window
     *
     * Describe window options for Envy to use when creating the window.
     * This is passed to Envy in a \ref Envy::engine::description object to
     * the \ref Envy::engine::run() function
     *
     * \see Envy::engine::description
     * \see Envy::engine::run()
     * \see Envy::graphics::description
     ********************************************************************************/
    struct description
    {
        std::wstring title {L"Envy Application"};          ///< Text displayed in the Window's Title Bar
        Envy::vector2<i32> minimum_window_size {300,300};  ///< Minimum width and height the window can be
        Envy::vector2<i32> maximum_window_size {0,0};      ///< Maximum width and height the window can be

        // TODO: border, resizable, confine cursor, etc
    };


    /********************************************************************************
     * \brief Pauses execution until Window is created
     *
     * Pauses execution of the current thred until the window thread has successfully
     * created the Window. Envy will call this in the \ref Envy::engine::run() function
     * to ensure the Window is created before initializing the graphics. You shouldn't
     * need to call this function yourself
     *
     * \see Envy::engine::run()
     ********************************************************************************/
    void wait_for_creation();


    /********************************************************************************
     * \brief Launches the Window thread
     *
     * Envy will call this function in the \ref Envy::engine::run() function.
     * Do not call this function yourself.
     *
     * \param [in] stop_token Used to request the thread spot executing
     * \param [in] windesc The window description forwarded from \ref Envy::engine::run()
     *
     * \see Envy::engine::run()
     ********************************************************************************/
    void run(std::stop_token stop_token, const description& windesc);


    /********************************************************************************
     * \brief Returns whether the window is open
     *
     * The Window is considered to be closed when the program has posted a WM_CLOSE message
     * is processed. You can use \ref Envy::window::request_close() to post such a message
     *
     * \return true if the Window hasn't been closed
     * \return false if the window has been closed
     *
     * \see Envy::window::request_close()
     ********************************************************************************/
    bool is_open();


    /********************************************************************************
     * \brief Requests the window be closed
     *
     * This will post a WM_CLOSE message. Because Envy only supports one window this
     * will cause the message loop to break and the Window thread to stop execution.
     * \ref Envy::engine::run() will detect this and return. execution will resume
     * in main.
     *
     * \see Envy::window::is_open()
     ********************************************************************************/
    void request_close();


    /********************************************************************************
     * \brief Returns the native Win32 window handle
     *
     * \return HWND the native Win32 window handle
     ********************************************************************************/
    HWND get_hwnd();


    /********************************************************************************
     * \brief Sets the Window's size constraints
     *
     * You can also set the window constraints in the \ref Envy::engine::description
     * you pass to \ref Envy::engine::run()
     *
     * \param [in] min Minimum width and height
     * \param [in] max Maximum width and height
     *
     * \see Envy::engine::description
     * \see Envy::window::description
     * \see Envy::vector2
     ********************************************************************************/
    void set_size_constraints(Envy::vector2<i32> min, Envy::vector2<i32> max);

    void request_fullscreen(bool full);
    bool is_fullscreen();
    Envy::vector2<i32> get_size();


    // ==== event types ====


    /********************************************************************************
     * \brief Window resized event
     *
     * Envy will post this event when the size of the window has been changed
     *
     * \see Envy::listener
     * \see Envy::register_callback()
     ********************************************************************************/
    class resized : public Envy::event
    {
    public:

        Envy::vector2<i32> size; ///< The new size of the Window

        resized(Envy::vector2<i32> s) : size {s} {}
    };


    /********************************************************************************
     * \brief Window resized event
     *
     * Envy will post this event when the size of the window is being changed
     *
     * \see Envy::listener
     * \see Envy::register_callback()
     ********************************************************************************/
    class resizing : public Envy::event
    {
    public:

        Envy::vector2<i32> size; ///< The new size of the Window

        resizing(Envy::vector2<i32> s) : size {s} {}
    };
}