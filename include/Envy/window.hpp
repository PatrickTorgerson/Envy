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

#pragma once

#include "common.hpp"

#include "log.hpp"
#include "vector.hpp"
#include "event.hpp"

#include <Windows.h>

#include <thread>

namespace Envy::window
{

    struct description
    {
        std::wstring title {L"Envy Application"};

        Envy::vector2<i32> minimum_window_size {300,300};
        Envy::vector2<i32> maximum_window_size {0,0};

        // TODO: border, resizable, confine cursor, etc
    };

    void wait_for_creation();

    void run(std::stop_token stop_token, const description& windesc);

    bool is_open();
    void request_close();

    HWND get_hwnd();

    void set_size_constraints(Envy::vector2<i32> min, Envy::vector2<i32> max);


    // event types


    class resized : public Envy::event
    {
    public:

        Envy::vector2<i32> size;

        resized(Envy::vector2<i32> s) : size {s} {}
    };

}